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

#include "qemu_icache.h"

#define DEBUG_TRACE
#define MODNAME "qemu-icache"
#include "utils/utils.h"


qemu_icache::qemu_icache(sc_module_name name, int nb_cpu)
                :sc_module(name)
{
    int i, line;
    num_cpu = nb_cpu;

    icache_tag = (unsigned long **)malloc(num_cpu *  sizeof (unsigned long *));
    for (i = 0; i < num_cpu; i++) {
        icache_tag[i] = (unsigned long *)malloc(ICACHE_LINES *  sizeof (unsigned long));
        for (line = 0; line < ICACHE_LINES; line++)
            icache_tag[i][line] = 0x8BADF00D;
    }
    icache_flags = (uint8_t **)malloc(num_cpu * sizeof (uint8_t *));
    for (i = 0; i < num_cpu; i++) {
        icache_flags[i] = (uint8_t *)malloc(ICACHE_LINES * sizeof (uint8_t));
        for (line = 0; line < ICACHE_LINES; line++) {
            icache_flags[i][line] = 0;
        }
    }

    nb_icache_access = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));
    nb_icache_miss = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));

    ns_miss = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));
    cumulate_ns_miss = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));

    for(i = 0; i < num_cpu; i++) {
        nb_icache_access[i] = 0;
        nb_icache_miss[i] = 0;
        ns_miss[i] = 0;
        cumulate_ns_miss[i] = 0;
    }
}

qemu_icache::~qemu_icache()
{
    int i;

    if(icache_tag) {
        for(i = 0; i < num_cpu; i++)
            if(icache_tag[i])
                delete icache_tag[i];
        delete icache_tag;
    }

    if(icache_flags) {
        for(i = 0; i < num_cpu; i++)
            if(icache_flags[i])
                delete icache_flags[i];
        delete icache_flags;
    }

    if(cumulate_ns_miss) {
        delete cumulate_ns_miss;
    }

    if(ns_miss)
         delete ns_miss;

    if(nb_icache_access)
         delete nb_icache_access;

    if(nb_icache_miss)
         delete nb_icache_miss;
}

void qemu_icache::info()
{
    int i;
    for(i=0;i<num_cpu;i++) {
        DPRINTF("number of access for CPU[%d] = %lu\n",i,nb_icache_access[i]);
        DPRINTF("number of miss for CPU[%d] = %lu\n",i,nb_icache_miss[i]);
#ifndef FULL_CACHE
        DPRINTF("time consumed for CPU[%d] = %lu\n",i,cumulate_ns_miss[i]);
#endif
    }

}

int qemu_icache::icache_line_present(int cpu, int start_idx, unsigned long tag)
{
    int idx, last_idx = start_idx + (1 << ICACHE_ASSOC_BITS);
    for (idx = start_idx; idx < last_idx; idx++)
        if (tag == icache_tag[cpu][idx])
            return idx;
    return -1;
}

int qemu_icache::icache_line_replace(int cpu, int start_idx)
{
    int idx, last_idx = start_idx + (1 << ICACHE_ASSOC_BITS);
    for (idx = start_idx; idx < last_idx; idx++)
        if (icache_flags[cpu][idx] == 0)
            return idx;
    return start_idx + (((1 << ICACHE_ASSOC_BITS) - 1) & random());
}


void qemu_icache::icache_access(int cpu, unsigned long addr,
                                uint32_t (*read_mem)(void *,uint32_t,uint32_t),void *opaque)
{
    unsigned long   tag = addr >> ICACHE_LINE_BITS;
    int             idx, start_idx;

    nb_icache_access[cpu]++;
    start_idx = tag & (ICACHE_LINES - 1) & ~((1 << ICACHE_ASSOC_BITS) - 1);
    idx = icache_line_present(cpu, start_idx, tag);

    if (idx == -1 || icache_flags[cpu][idx] == 0) {
        nb_icache_miss[cpu]++;

        idx = icache_line_replace(cpu, start_idx);

        icache_tag[cpu][idx] = tag;
        icache_flags[cpu][idx] = 1;

#ifdef FULL_CACHE
        //read from memory (the full cache configuration)
        read_mem(opaque,0x00000004,4);
        /*we don't care about the addr since we just need to do a mem read
          so we only need to be sure it in the RAM
          we also don't care about the data size */
#else
        //calculate cycles (the late cache configuration)
        ns_miss[cpu] += NS_ICACHE_MISS;
        cumulate_ns_miss[cpu] += NS_ICACHE_MISS;
        if(ns_miss[cpu] > NS_ICACHE_MISS * MAX_ICACHE_MISS) {
            wait(ns_miss[cpu],SC_NS);
            ns_miss[cpu] = 0;
        }
#endif
    }

}


