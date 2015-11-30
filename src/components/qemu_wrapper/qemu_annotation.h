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

#ifndef _QEMU_WAPPER_QEMU_ANNOTATION_H
#define _QEMU_WAPPER_QEMU_ANNOTATION_H

#include "rabbits-common.h"

#define MAX_CYCLES 20000

class qemu_annotation : public sc_module {
private:
    unsigned long *nb_cycles;
    unsigned long *cumulate_nb_cycles;
    int num_cpu;

public:
    qemu_annotation(sc_module_name name,int cpu);
    ~qemu_annotation();

    void update_cpu_cycles(unsigned long cycles, int current_cpu);
    void consume_cpu_cycles(int current_cpu);
    void info(void);

};

#endif