/*
 *  This file is part of Rabbits
 *  Copyright (C) 2015  Clement Deschamps and Luc Michel
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "qemu_dcache.h"

#define DEBUG_TRACE
#define MODNAME "qemu-dcache"
#include "utils/utils.h"


qemu_dcache::qemu_dcache(sc_module_name name, int nb_cpu)
                :sc_module(name)
{
    int i, line, w;
    num_cpu = nb_cpu;

    dcache_tag = (unsigned long **)malloc(num_cpu *  sizeof (unsigned long *));
    for (i = 0; i < num_cpu; i++) {
        dcache_tag[i] = (unsigned long *)malloc(DCACHE_LINES *  sizeof (unsigned long));
        for (line = 0; line < DCACHE_LINES; line++)
            dcache_tag[i][line] = 0x8BADF00D;
    }
    dcache_flags = (d_flags **)malloc(num_cpu * sizeof (d_flags *));
    for (i = 0; i < num_cpu; i++) {
        dcache_flags[i] = (d_flags *)malloc(DCACHE_LINES * sizeof (d_flags));
        for (line = 0; line < DCACHE_LINES; line++) {
            dcache_flags[i][line].dirty = 0;
            dcache_flags[i][line].state = INVALID;
        }
    }

    dcache_data = (uint8_t ***)malloc(num_cpu *  sizeof (uint8_t **));
    for (i = 0; i < num_cpu; i++) {
        dcache_data[i] = (uint8_t **)malloc(DCACHE_LINES *  sizeof (uint8_t *));
        for (line = 0; line < DCACHE_LINES; line++) {
            dcache_data[i][line] = (uint8_t *)malloc(DCACHE_LINE_SIZE *  sizeof (uint8_t));
            for (w = 0; w < DCACHE_LINE_WORDS; w++)
                *( ((uint32_t *)dcache_data[i][line]) + w) = (uint32_t)0xDEADBEEF;
        }
    }

    nb_dcache_miss = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));

    nb_mem_read = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));
    nb_mem_write = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));

    ns_dcache = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));
    cumulate_ns_dcache = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));

    for(i = 0; i < num_cpu; i++) {
        nb_dcache_miss[i] = 0;
        nb_mem_read[i] = 0;
        nb_mem_write[i] = 0;
        ns_dcache[i] = 0;
        cumulate_ns_dcache[i] = 0;
    }
}

qemu_dcache::~qemu_dcache()
{
    int i;

    if(dcache_tag) {
        for(i = 0; i < num_cpu; i++)
            if(dcache_tag[i])
                delete dcache_tag[i];
        delete dcache_tag;
    }

    if(dcache_flags) {
        for(i = 0; i < num_cpu; i++)
            if(dcache_flags[i])
                delete dcache_flags[i];
        delete dcache_flags;
    }

    if(cumulate_ns_dcache) {
        delete cumulate_ns_dcache;
    }

    if(ns_dcache)
         delete ns_dcache;

    if(nb_mem_read)
         delete nb_mem_read;

    if(nb_mem_write)
         delete nb_mem_write;

    if(nb_dcache_miss)
         delete nb_dcache_miss;
}

void qemu_dcache::info()
{

    int i;
    for(i=0;i<num_cpu;i++) {
        DPRINTF("number of mem read for CPU[%d] = %lu\n",i,nb_mem_read[i]);
        DPRINTF("number of cache miss for CPU[%d] = %lu\n",i,nb_dcache_miss[i]);
        DPRINTF("number of mem write for CPU[%d] = %lu\n",i,nb_mem_write[i]);
#ifndef FULL_CACHE
        DPRINTF("time consumed for CPU[%d] = %lu\n",i,cumulate_ns_dcache[i]);
#endif
    }

}

void qemu_dcache::consume_cpu_cycles(void)
{
#ifndef WAIT_CACHE
    unsigned long max = 0;
    int i;

    for(i = 0; i < num_cpu; i++)
        if(ns_dcache[i] > max)
            max = ns_dcache[i];

    wait(max,SC_NS);

    for(i = 0; i < num_cpu; i++)
        ns_dcache[i] = 0;
#endif
}



int qemu_dcache::dcache_line_present(int cpu, int start_idx, unsigned long tag)
{
    int idx, last_idx = start_idx + (1 << DCACHE_ASSOC_BITS);
    for (idx = start_idx; idx < last_idx; idx++)
        if (tag == dcache_tag[cpu][idx])
            return idx;
    return -1;
}

int qemu_dcache::dcache_line_replace(int cpu, int start_idx)
{
    int idx, last_idx = start_idx + (1 << DCACHE_ASSOC_BITS);
    for (idx = start_idx; idx < last_idx; idx++)
        if (dcache_flags[cpu][idx].state == INVALID)
            return idx;
    return start_idx + (((1 << DCACHE_ASSOC_BITS) - 1) & random());
}

void * qemu_dcache::dcache_read(int cpu, unsigned long addr,
                        uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                        void (*write_mem)(void *,uint32_t, uint32_t,uint32_t),void *opaque)
{
    int idx, start_idx, w;
    unsigned long tag;

    tag = addr >> DCACHE_LINE_BITS;
    start_idx = tag & (DCACHE_LINES - 1) & ~((1 << DCACHE_ASSOC_BITS) - 1);
    idx = dcache_line_present(cpu, start_idx, tag);

//    DPRINTF("CPU[%d] Cache Read for addr  = %08x\n",cpu,addr);
    if (idx == -1 || dcache_flags[cpu][idx].state == INVALID) { /* cache miss */
        nb_dcache_miss[cpu]++;
//        DPRINTF("\tCPU[%d] Cache miss for addr  = %08x\n",cpu,addr);

        idx = dcache_line_replace(cpu, start_idx);
        dcache_tag[cpu][idx] = tag;
        dcache_flags[cpu][idx].state = SHARED;

        //If a processor own the data (state = Modified) we should update it to Shared
        //Only one CPU can have the Modified state
        // --> At the end we only have Shared state for this data in all the CPU
        int c, ic;
        for(c = 0; c < num_cpu && c != cpu; c++) {
            ic = dcache_line_present(c, start_idx, tag);
            if(ic != -1 && dcache_flags[c][ic].state == MODIFIED) {
                dcache_flags[c][ic].state = SHARED;
//                DPRINTF("\t\t @ CPU[%d] Modified to Shared for addr  = %08x\n",c,addr);
            }
        }

#ifdef FULL_CACHE
        //perform a mem access
        unsigned long addr_read = addr & ~DCACHE_LINE_MASK;
        for (w = 0; w < DCACHE_LINE_WORDS; w++)
            *(  ((uint32_t *)dcache_data[cpu][idx]) + w) = read_mem(opaque, addr_read + 4*w, 4);
#else
    #ifndef WAIT_CACHE
        //calculate cycles (the late cache configuration)
        ns_dcache[cpu] += NS_DCACHE_READ;
    #else
        //perform a wait (wait cache configuration
        wait(NS_DCACHE_READ,SC_NS);
    #endif
        cumulate_ns_dcache[cpu] += NS_DCACHE_READ;
#endif
     }

    return &dcache_data[cpu][idx][addr & DCACHE_LINE_MASK]; //not needed in cache Late
}

uint64_t qemu_dcache::dcache_read_q(int cpu, unsigned long addr,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    uint32_t low, hi;

    if(count == 1) {//here we are sure it will be always 1 cause we cannot call recursivly the read_q
        nb_mem_read[cpu]++;
//        DPRINTF("CPU[%d] Reading 64bits @ %08x\n",cpu,addr);
    }

    low = dcache_read_l(cpu, addr, read_mem, write_mem, opaque, 0);
    hi  = dcache_read_l(cpu, addr + 4, read_mem, write_mem, opaque, 0);

    return (((uint64_t) hi) << 32) + low;
}

uint32_t qemu_dcache::dcache_read_l(int cpu, unsigned long addr,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    if(count == 1) {
        nb_mem_read[cpu]++;
//        DPRINTF("CPU[%d] Reading 32bits @ %08x\n",cpu,addr);
    }

    if ( ((addr & DCACHE_LINE_MASK) + 3) < DCACHE_LINE_SIZE ) /* We are not at a cache line boundary so we don't care about alignement*/
        return *(uint32_t *)dcache_read(cpu, addr, read_mem, write_mem, opaque);
    else { /* Here we are at a cache line boundary, we have 2 cases*/
        uint32_t x, y, z;
//        DPRINTF("Reading 32bits not aligned\n");
        if (addr & 1) {
            z = dcache_read_b(cpu, addr + 0, read_mem, write_mem, opaque, 0);
            y = dcache_read_w(cpu, addr + 1, read_mem, write_mem, opaque, 0); /* half word aligned for sure, next cache line if 3 */
            x = dcache_read_b(cpu, addr + 3, read_mem, write_mem, opaque, 0); /* on next cache line */
            return (x << 24) | (y << 8) | z;
        } else {
            y = dcache_read_w(cpu, addr + 0, read_mem, write_mem, opaque, 0); /* half word aligned for sure */
            x = dcache_read_w(cpu, addr + 2, read_mem, write_mem, opaque, 0); /* half word aligned on next cache line */
            return (x << 16) | y;
        }
    }
}

uint16_t qemu_dcache::dcache_read_w(int cpu, unsigned long addr,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    if(count == 1) {
        nb_mem_read[cpu]++;
//        DPRINTF("CPU[%d] Reading 16bits @ %08x\n",cpu,addr);
    }

    if ( ((addr & DCACHE_LINE_MASK) + 1) < DCACHE_LINE_SIZE ) /* We are not at a cache line boundary */
        return *(uint16_t *)dcache_read(cpu, addr, read_mem, write_mem, opaque);
    else {/* Here we are at a cache line boundary, we have only 1 case */
        uint8_t x, y;
        y = dcache_read_b(cpu, addr + 0, read_mem, write_mem, opaque, 0);
        x = dcache_read_b(cpu, addr + 1, read_mem, write_mem, opaque, 0);
//        DPRINTF("Reading 16bits not aligned [%02x %02x]\n",x,y);
        return (x << 8) | y;
    }
}

uint8_t qemu_dcache::dcache_read_b(int cpu, unsigned long addr,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    if(count == 1) {
        nb_mem_read[cpu]++;
//        DPRINTF("CPU[%d] Reading 8bits @ %08x\n",cpu,addr);
    }

    return *(uint8_t *)dcache_read(cpu, addr, read_mem, write_mem, opaque);
}



void qemu_dcache::dcache_write(int cpu, unsigned long addr, int nb, uint32_t val,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque)
{

    unsigned long tag = addr >> DCACHE_LINE_BITS;
    unsigned long ofs = addr & DCACHE_LINE_MASK;
    int idx, start_idx;

    start_idx = tag & (DCACHE_LINES - 1) & ~((1 << DCACHE_ASSOC_BITS) - 1);
    idx = dcache_line_present(cpu, start_idx, tag);

//    DPRINTF("CPU[%d] Cache write for addr  = %08x with = %08x\n",cpu,addr,val);
    if (idx == -1 || dcache_flags[cpu][idx].state == INVALID) { /* cache miss  */
        //we work with allocate on write, so in all the cases we will write on cache and mem
//        DPRINTF("\tCPU[%d] Cache miss for addr  = %08x\n",cpu,addr);
        idx = dcache_line_replace(cpu, start_idx);
        dcache_tag[cpu][idx] = tag;
    }
    if(dcache_flags[cpu][idx].state != MODIFIED) //we don't own the data (cache miss or state = shared)
    {
        dcache_flags[cpu][idx].state = MODIFIED;
        //we need to invalidate the other caches
        int c, ic;
        for(c = 0; c < num_cpu && c != cpu; c++) {
            ic = dcache_line_present(c, start_idx, tag);
            if(ic != -1 && dcache_flags[c][ic].state != INVALID) //maybe we will find one Modifed or many Shared
                dcache_flags[c][ic].state = INVALID;
        }

    }

    //we update the value in the cache
    //we do it but actually we don't need
    // for late cache we don't really need the data and for full cache we are going to reload the whole line again

    switch(nb) {
    case 1:
        *((int8_t *)&dcache_data[cpu][idx][ofs]) = (int8_t)(val & 0x000000ff);
        break;
    case 2:
        *((int16_t *)&dcache_data[cpu][idx][ofs]) = (int16_t)(val & 0x0000ffff);
        break;
    case 4:
        *((int32_t *)&dcache_data[cpu][idx][ofs]) = (int32_t)(val & 0xffffffff);
        break;
    }


#ifdef FULL_CACHE
    //we write to the mem
//    write_mem(opaque, addr, /*dcache_data[cpu][idx][ofs]*/val, nb);
    //we bring the whole line from Mem
    unsigned long addr_read = addr & ~DCACHE_LINE_MASK;
    int w;
    for (w = 0; w < DCACHE_LINE_WORDS; w++) {
        *(  ((uint32_t *)dcache_data[cpu][idx]) + w) = read_mem(opaque, addr_read + 4*w, 4);
//        DPRINTF("\tCache filling line @ addr  = %08x with = %08x\n",addr_read + 4*w,*(  ((uint32_t *)dcache_data[cpu][idx]) + w));
   }
#else
    #ifndef WAIT_CACHE
    //calculate cycles (the late cache configuration)
    ns_dcache[cpu] += NS_DCACHE_WRITE;
    #else
    //perform a wait (wait cache configuration)
    wait(NS_DCACHE_WRITE,SC_NS);
    #endif
    cumulate_ns_dcache[cpu] += NS_DCACHE_WRITE;
#endif


}

void qemu_dcache::dcache_write_q(int cpu, unsigned long addr, uint64_t val,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    if(count == 1) {
        nb_mem_write[cpu]++;
//        DPRINTF("CPU[%d] Writing 64bits @ %08x\n",cpu,addr);
    }

    dcache_write_l(cpu, addr + 0, (uint32_t)(val & 0xffffffff), read_mem, write_mem, opaque, 0);
    dcache_write_l(cpu, addr + 4, (uint32_t)(val >> 32), read_mem, write_mem, opaque, 0);
}

void qemu_dcache::dcache_write_l(int cpu, unsigned long addr, uint32_t val,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    if(count == 1) {
        nb_mem_write[cpu]++;
//        DPRINTF("CPU[%d] Writing 32bits @ %08x\n",cpu,addr);
    }

    if (( (addr & DCACHE_LINE_MASK) + 3) < DCACHE_LINE_SIZE) {
        dcache_write(cpu, addr, 4, (uint32_t)val, read_mem, write_mem, opaque);
    } else { /* Unaligned*/
//        DPRINTF("Writing 32bits not aligned\n");
        if (addr & 1) {
            dcache_write_b(cpu, addr + 0, (uint8_t)(val >> 0) /*& 0x000000ff*/, read_mem, write_mem, opaque, 0);
            dcache_write_w(cpu, addr + 1, (uint16_t)(val >> 8) /*& 0x0000ffff*/, read_mem, write_mem, opaque, 0);
            dcache_write_b(cpu, addr + 3, (uint8_t)(val >> 24) /*& 0x000000ff*/, read_mem, write_mem, opaque, 0);
        } else {
            dcache_write_w(cpu, addr + 0, (uint16_t)(val >> 0)  /*& 0x0000ffff*/, read_mem, write_mem, opaque, 0);
            dcache_write_w(cpu, addr + 2, (uint16_t)(val >> 16) /*& 0x0000ffff*/, read_mem, write_mem, opaque, 0);
        }
   }

}

void qemu_dcache::dcache_write_w(int cpu, unsigned long addr, uint16_t val,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    if(count == 1) {
        nb_mem_write[cpu]++;
//        DPRINTF("CPU[%d] Writing 8bits @ %08x\n",cpu,addr);
    }

    if (((addr & DCACHE_LINE_MASK) + 1) < DCACHE_LINE_SIZE) {
        dcache_write(cpu, addr, 2, (uint32_t)val, read_mem, write_mem, opaque);
    } else { /* Unaligned */
//        DPRINTF("Writing 16bits not aligned\n");
        dcache_write_b(cpu, addr + 0, (uint8_t)(val >> 0) /*& 0x000000ff*/, read_mem, write_mem, opaque, 0);
        dcache_write_b(cpu, addr + 1, (uint8_t)(val >> 8) /*& 0x000000ff*/, read_mem, write_mem, opaque, 0);
    }


}


void qemu_dcache::dcache_write_b(int cpu, unsigned long addr, uint8_t val,
                    uint32_t (*read_mem)(void *,uint32_t, uint32_t),
                    void (*write_mem)(void *,uint32_t, uint32_t, uint32_t),void *opaque,
                    int count)
{
    if(count == 1) {
        nb_mem_write[cpu]++;
//        DPRINTF("CPU[%d] Writing 8bits @ %08x\n",cpu,addr);
    }

    dcache_write(cpu, addr, 1, (uint32_t)val, read_mem, write_mem, opaque);
}
