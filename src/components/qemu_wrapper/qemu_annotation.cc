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

#include "qemu_annotation.h"

#define DEBUG_TRACE
#define MODNAME "qemu-annotation"
#include "utils/utils.h"


qemu_annotation::qemu_annotation(sc_module_name name, int nb_cpu)
                :sc_module(name)
{
    int i;
    num_cpu = nb_cpu;
    nb_cycles = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));
    cumulate_nb_cycles = (unsigned long *)malloc(num_cpu * sizeof(unsigned long));
    for(i = 0; i < num_cpu; i++) {
        nb_cycles[i] = 0;
        cumulate_nb_cycles[i] = 0;
    }
}

qemu_annotation::~qemu_annotation()
{
    int i;
    if(cumulate_nb_cycles) {
        delete cumulate_nb_cycles;
    }
    if(nb_cycles)
         delete nb_cycles;
}

void qemu_annotation::info()
{
    int i;
    if(cumulate_nb_cycles) {
        for(i = 0; i < num_cpu; i++) {
           DPRINTF("NB Cycles for CPU[%d] = %lu\n",i,cumulate_nb_cycles[i]);
           DPRINTF("NB Cycles remaining for CPU[%d] = %lu\n",i,nb_cycles[i]);
        }
        DPRINTF("TIME = %lu ns\n",sc_time_stamp().value ()/1000);
    }
}

void qemu_annotation::update_cpu_cycles(int cpu, unsigned long cycles)
{
    cumulate_nb_cycles[cpu] += cycles;
    nb_cycles[cpu] += cycles;
    if(nb_cycles[cpu] > MAX_CYCLES)
        consume_cpu_cycles(cpu);
}
void qemu_annotation::consume_cpu_cycles(int cpu)
{
//    DPRINTF("Consumming %lu for CPU %d\n",nb_cycles[current_cpu],current_cpu);
    //TODO : calculate the time we should really wait
    wait(nb_cycles[cpu],SC_NS);
    nb_cycles[cpu] = 0;

}
