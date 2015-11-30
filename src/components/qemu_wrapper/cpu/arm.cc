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

#include <sc_qemu.h>

#include "arm.h"

qemu_cpu_arm::qemu_cpu_arm(sc_module_name n, int cpuid, qemu_lib_wrapper *lib) 
    : qemu_cpu(n, cpuid, lib)
{
}

sc_in<bool> & qemu_cpu_arm::get_line_irq()
{
    /* Mapping with indexes in QEMU
     *
     * We have sc_in<bool> in SystemC world and indexes in QEMU world.
     * <sc_qemu.h> exposes theses indexes as constants so we know how to map
     * them to the corresponding sc_in here.
     *
     * This mapping is then used by the parent class qemu_device_interruptable
     * to update QEMU irq when a sc_in state changes 
     *
     * the get_irq protected method will create the sc_in port on the first call
     */
    return get_irq(SC_QEMU_ARM_IRQ_IRQ);
}

sc_in<bool> & qemu_cpu_arm::get_line_fiq()
{
    return get_irq(SC_QEMU_ARM_IRQ_FIQ);
}
