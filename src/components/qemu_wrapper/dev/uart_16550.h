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

#ifndef _QEMU_WRAPPER_DEV_UART_16550_H
#define _QEMU_WRAPPER_DEV_UART_16550_H

#include "qemu_device.h"

class qemu_uart_16550 : public qemu_device
{
protected:
    uint32_t m_base_addr;
    int m_baudbase;
    int m_irq_idx;
    int m_regshift;

    bool m_valid_base_addr, m_valid_baudbase;
    bool m_valid_irq_idx, m_valid_regshift, m_realized;

    bool realizable()
    {
        return m_valid_base_addr 
            && m_valid_baudbase 
            && m_valid_irq_idx
            && m_valid_regshift
            && (m_int_ctrl != NULL)
            && !m_realized;
    }

    void realize();
    virtual void end_of_elaboration();

public:
    qemu_uart_16550(sc_module_name n, qemu_lib_wrapper *lib);
    virtual ~qemu_uart_16550() {}

    void set_base_addr(uint32_t base_addr)
    { 
        m_base_addr = base_addr;
        m_valid_base_addr = true;
        realize();
    }

    void set_baudbase(int baudbase)
    { 
        m_baudbase = baudbase; 
        m_valid_baudbase = true;
        realize();
    }

    void connect_irq(int idx)
    {
        m_irq_idx = idx;
        m_valid_irq_idx = true;
        realize();
    }

    void set_regshift(int regshift)
    {
        m_regshift = regshift;
        m_valid_regshift = true;
        realize();
    }

};

#endif
