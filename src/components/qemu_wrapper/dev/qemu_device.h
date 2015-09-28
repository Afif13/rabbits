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

/* 
 * Generic QEMU device
 */

#ifndef _QEMU_WRAPPER_DEV_QEMU_DEVICE_H
#define _QEMU_WRAPPER_DEV_QEMU_DEVICE_H

#include <map>
#include <systemc.h>

#include "../qemu_lib_wrapper.h"

#define MODNAME "qemu-device"
#include "utils/utils.h"

class qemu_device_interruptable;

class qemu_device : public sc_module
{
protected:
    qemu_lib_wrapper *m_lib;
    sc_qemu_qdev     *m_dev;
    qemu_device_interruptable *m_int_ctrl;

public:
    qemu_device(sc_module_name n, qemu_lib_wrapper *lib)
        : sc_module(n)
    {
        m_lib = lib;
        m_int_ctrl = NULL;
    }

    void set_int_ctrl(qemu_device_interruptable *int_ctrl) { m_int_ctrl = int_ctrl; }

    virtual ~qemu_device() {}

    sc_qemu_qdev * get_qdev() { return m_dev; }
};

/*
 * A generic QEMU device with input interrupt lines
 * (an interrupt controller or a cpu)
 */
class qemu_device_interruptable : public qemu_device
{
protected:
    std::map<int, sc_in<bool>* > m_irq;
    sc_event_or_list m_irq_events;

    void irq_thread() 
    {
        std::map<int, sc_in<bool>* >::iterator it;

        if (!m_irq.size()) {
            return;
        }

        for (;;) {
            wait(m_irq_events);

            for (it = m_irq.begin(); it != m_irq.end(); it++) {
                int irq_idx = it->first;
                sc_in<bool> * irq_line = it->second;
                
                if (irq_line->posedge()) {
                    m_lib->qdev_irq_update(m_dev, irq_idx, 1);
                } else if (irq_line->negedge()) {
                    m_lib->qdev_irq_update(m_dev, irq_idx, 0);
                }
            }
        }
    }

    void end_of_elaboration()
    {
        std::map<int, sc_in<bool>* >::iterator it;

        for (it = m_irq.begin(); it != m_irq.end(); it++) {
            sc_in<bool> * irq_line = it->second;

            m_irq_events |= irq_line->default_event();
        }
    }

    virtual bool check_irq_idx(int idx) = 0;

public:
    SC_HAS_PROCESS(qemu_device_interruptable);

    qemu_device_interruptable(sc_module_name n, qemu_lib_wrapper *lib)
        : qemu_device(n, lib)
    {
        SC_THREAD(irq_thread);
    }

    virtual ~qemu_device_interruptable() {}

    sc_in<bool> & get_irq(int idx)
    {
        if (!check_irq_idx(idx)) {
            EPRINTF("Trying to access invalid irq index %d of component %s\n", idx, name());
            exit(1);
        }

        if (m_irq.find(idx) == m_irq.end()) {
            /* Dynamic IRQ creation */
            std::stringstream ss;
            ss << "irq" << idx;
            sc_in<bool> * new_irq = new sc_in<bool>(ss.str().c_str());
            m_irq[idx] = new_irq;

        }

        return *(m_irq[idx]);
    }
};

#undef MODNAME
#endif
