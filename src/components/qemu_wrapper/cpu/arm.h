/*
 *  Copyright (c) 2015 Clement Deschamps and Luc Michel
 *
 *  This file is part of Rabbits.
 *
 *  Rabbits is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Rabbits is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Rabbits.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _QEMU_WRAPPER_QEMU_CPU_ARM_H
#define _QEMU_WRAPPER_QEMU_CPU_ARM_H

#include <string>

#include "qemu_cpu.h"

class qemu_lib_wrapper;

class qemu_cpu_arm : public qemu_cpu {
public:
    qemu_cpu_arm(sc_module_name n, int cpuid, qemu_lib_wrapper *lib);

    static std::string get_iss_lib() { return "libqemu-system-arm.so"; }

    virtual bool check_irq_idx(int idx) { return idx < 2; }

    /* ARM interrupt lines */
    sc_in<bool> & get_line_irq();
    sc_in<bool> & get_line_fiq();
};

#endif
