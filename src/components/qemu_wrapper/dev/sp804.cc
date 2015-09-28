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

#include "sp804.h"

qemu_sp804::qemu_sp804(sc_module_name n, qemu_lib_wrapper *lib)
    : qemu_device(n, lib)
{
    m_dev = m_lib->qdev_create_sp804();
}

void qemu_sp804::map_mmio(uint32_t addr)
{
    m_lib->qdev_mmio_map(m_dev, 0, addr);
}

void qemu_sp804::connect_irq(int idx)
{
    if(m_int_ctrl != NULL) {
        m_lib->qdev_irq_connect(m_dev, 0, m_int_ctrl->get_qdev(), idx);
    }
}

