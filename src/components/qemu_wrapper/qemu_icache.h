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

#ifndef _QEMU_WAPPER_QEMU_ICACHE_H
#define _QEMU_WAPPER_QEMU_ICACHE_H

#include "rabbits-common.h"

//#define FULL_CACHE

#define ICACHE_LINES 1024
#define ICACHE_ASSOC_BITS 1
#define ICACHE_LINE_BITS 5

//Time we should wait when we have a miss and will perform mem access (for the late cache configuration)
#define NS_ICACHE 10


class qemu_icache : public sc_module {
private:
    uint8_t (**icache_flags);
    unsigned long (**icache_tag);
    unsigned long *nb_icache_access;
    unsigned long *nb_icache_miss;

    unsigned long *ns_icache;
    unsigned long *cumulate_ns_icache;

    int num_cpu;

public:
    qemu_icache(sc_module_name name,int nb_cpu);
    ~qemu_icache();

    void icache_access(int cpu, unsigned long addr,
                        uint32_t (*)(void *,uint32_t, uint32_t),void *);

    void info(void);
    void consume_cpu_cycles(int cpu);

private:
    int icache_line_present(int cpu, int start_idx, unsigned long tag);
    int icache_line_replace(int cpu, int start_idx);
};

#endif
