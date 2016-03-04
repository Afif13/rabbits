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

#ifndef _QEMU_WAPPER_QEMU_DCACHE_H
#define _QEMU_WAPPER_QEMU_DCACHE_H

#include "rabbits-common.h"
#include "qemu-cache.h"

class qemu_dcache : public sc_module {
private:
    d_flags (**dcache_flags);
    unsigned long (**dcache_tag);
    uint8_t (***dcache_data);

    unsigned long *nb_dcache_miss;

    unsigned long *nb_mem_read;
    unsigned long *nb_mem_write;

    unsigned long *ns_dcache;
    unsigned long *cumulate_ns_dcache;

    int num_cpu;

public:
    qemu_dcache(sc_module_name name,int nb_cpu);
    ~qemu_dcache();

    uint8_t dcache_read_b(int cpu, unsigned long addr,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count); /*since we are calling recursivly our functions
                                    we use this var to avoid over counting*/

    uint16_t dcache_read_w(int cpu, unsigned long addr,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count);

    uint32_t dcache_read_l(int cpu, unsigned long addr,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count);

    uint64_t dcache_read_q(int cpu, unsigned long addr,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count);



    void dcache_write_b(int cpu, unsigned long addr, uint8_t val,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count);

    void dcache_write_w(int cpu, unsigned long addr, uint16_t val,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count);

    void dcache_write_l(int cpu, unsigned long addr, uint32_t val,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count);

    void dcache_write_q(int cpu, unsigned long addr, uint64_t val,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *,
                        int count);

    void info(void);
    void consume_cpu_cycles(void);


private:
    int dcache_line_present(int cpu, int start_idx, unsigned long tag);
    int dcache_line_replace(int cpu, int start_idx);

    void * dcache_read(int cpu, unsigned long addr,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *);

    void dcache_write(int cpu, unsigned long addr, int nb, uint32_t val,
                        uint32_t (*)(void *,uint32_t, uint32_t),
                        void (*)(void *,uint32_t, uint32_t, uint32_t),void *);

};

#endif
