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

#include "uart_16550.h"

#define MODNAME "qemu-uart-16550"
#include "utils/utils.h"

qemu_uart_16550::qemu_uart_16550(sc_module_name n, qemu_lib_wrapper *lib)
    : qemu_device(n, lib)
{
    m_valid_base_addr = m_valid_baudbase = m_valid_irq_idx = false;
    m_regshift = 2;
}

void qemu_uart_16550::realize()
{
    if(realizable()) {
        m_lib->qdev_create_uart_16550(m_base_addr, m_regshift, m_int_ctrl->get_qdev(),
                                      m_irq_idx, m_baudbase);
        m_realized = true;
    }
}

void qemu_uart_16550::end_of_elaboration() {
    if(!m_realized) {
        EPRINTF("%s has not been realized! missing: %s%s%s%s%s\n", 
                name(),
                m_valid_base_addr ? "" : "base address  ",
                m_valid_baudbase ? "" : "baud base  ",
                m_valid_irq_idx ? "" : "interrupt mapping  ",
                m_valid_regshift ? "" : "register shift",
                m_int_ctrl ? "" : "interrupt controller");
    }
}

