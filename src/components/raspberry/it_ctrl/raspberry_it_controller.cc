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

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "raspberry_it_controller.h"

#define MODNAME "rpi-it-ctrl"
#include "utils/utils.h"

#define TIMER_CLOCK_FV 1000000000

raspberry_it_controller::raspberry_it_controller(sc_module_name module_name,
        int it_ports, sc_signal<bool>* irq_wires, int* irq_wires_num,
        int it_ap_ports, sc_signal<bool>* irq_ap_wires, int* irq_ap_wires_num) :
        Slave(module_name), m_it_ports(it_ports), m_it_ap_ports(
                it_ap_ports)
{
    if (it_ports > 0) {
        /* Create input ports and bind wires to theses inputs */
        irq_in = new sc_in<bool> [it_ports];
        irq_num = new int[it_ports];

        for (int i = 0; i < it_ports; i++) {
            irq_in[i](irq_wires[i]);
            irq_num[i] = irq_wires_num[i];
            /* The interrupt number must be coherent to avoid seg faults */
            assert(
                    (irq_wires_num[i] >= 0) && (irq_wires_num[i] < IT_CONT_PORT_NB));
            m_event_list |= irq_in[i].value_changed_event();
            //m_event_list |= irq_in[i].default_event();
        }
    }

    if (it_ap_ports > 0) {
        /* Create input ports and bind wires to theses inputs */
        irq_ap_in = new sc_in<bool> [it_ap_ports];
        irq_ap_num = new int[it_ap_ports];

        for (int i = 0; i < it_ap_ports; i++) {
            irq_ap_in[i](irq_ap_wires[i]);
            irq_ap_num[i] = irq_ap_wires_num[i];
            /* The interrupt number must be coherent to avoid seg faults */
            assert(
                    (irq_ap_wires_num[i] >= 0) && (irq_ap_wires_num[i] < IT_CONT_AP_PORT_NB));
            m_event_list |= irq_ap_in[i].value_changed_event();
            //m_event_list |= irq_ap_in[i].default_event();
        }
    }
    m_event_list |= ev_refresh;
    reset_registers();

    SC_THREAD(it_thread);
}

raspberry_it_controller::~raspberry_it_controller()
{
}

void raspberry_it_controller::bus_cb_write_32(uint64_t ofs, uint32_t *data, bool &bErr)
{
    uint32_t val1 = *data;
    bool refresh = false;

    DPRINTF("write addr = 0x%" PRIx64 ", val = 0x%" PRIx32 " \n", ofs, val1);
    switch (ofs) {
    case IT_CONT_ENABLE_IRQ_1:
        for (int i = 0; i < 32; i++) {
            if (val1 & (1 << i)) {
                m_it_enable_reg[i] = 1;
                refresh = true;
            }
        }
        DPRINTF("ENABLE_IRQ_1 = %" PRIx32 " \n", val1);
        break;

    case IT_CONT_ENABLE_IRQ_2:
        for (int i = 0; i < 32; i++) {
            if (val1 & (1 << i)) {
                /* We are enabeling bit 32 to 63 here */
                m_it_enable_reg[i + 32] = 1;
                refresh = true;
            }
        }
        DPRINTF("ENABLE_IRQ_2 = %" PRIx32 " \n", val1);
        break;

    case IT_CONT_ENABLE_BASIC_IRQ:
        for (int i = 0; i < 8; i++) {
            if (val1 & (1 << i)) {
                /* We are enabeling bit 0 to 7 here */
                m_it_ap_enable_reg[i] = 1;
                refresh = true;
            }
            DPRINTF("ENABLE_BASIC_IRQ = %" PRIx32 " \n", val1);
        }
        break;

    case IT_CONT_DISABLE_IRQ_1:
        for (int i = 0; i < 32; i++) {
            if (val1 & (1 << i)) {
                m_it_enable_reg[i] = 0;
                refresh = true;
            }
        }
        break;

    case IT_CONT_DISABLE_IRQ_2:
        for (int i = 0; i < 32; i++) {
            if (val1 & (1 << i)) {
                m_it_enable_reg[i + 32] = 0;
                refresh = true;
            }
        }
        break;

    case IT_CONT_DISABLE_BASIC_IRQ:
        for (int i = 0; i < 8; i++) {
            if (val1 & (1 << i)) {
                m_it_ap_enable_reg[i] = 0;
                refresh = true;
            }
        }
        break;

    default:
        printf("Bad %s::%s ofs=0x%" PRIx64 " , data=0x%" PRIx32 " -%" PRIx32 " !\n", name(),
                __FUNCTION__, ofs,
                *((uint32_t *) data + 0),
                *((uint32_t *) data + 1));
        exit(1);
        break;
    }
    if (refresh) {
        ev_refresh.notify();
    }
    bErr = false;
}

void raspberry_it_controller::bus_cb_read_32(uint64_t ofs, uint32_t *data, bool &bErr)
{
    uint32_t out = 0;
    uint32_t pending_in_r2, pending_in_r1;

    DPRINTF("read addr = 0x%" PRIx64 " \n", ofs);

    switch (ofs) {
    case IT_CONT_ENABLE_IRQ_1:
        for (int i = 0; i < 32; i++) {
            out |= m_it_enable_reg[i] << i;
        }
        *data = out;
        break;

    case IT_CONT_ENABLE_IRQ_2:
        for (int i = 0; i < 32; i++) {
            out |= m_it_enable_reg[i + 32] << i;
        }
        *data = out;
        break;

    case IT_CONT_ENABLE_BASIC_IRQ:
        for (int i = 0; i < 8; i++) {
            out |= m_it_ap_enable_reg[i] << i;
        }
        *data = out;
        break;

    case IT_CONT_DISABLE_IRQ_1:
        for (int i = 0; i < 32; i++) {
            out |= !m_it_enable_reg[i] << i;
        }
        *data = out;
        break;

    case IT_CONT_DISABLE_IRQ_2:
        for (int i = 0; i < 32; i++) {
            out |= !m_it_enable_reg[i + 32] << i;
        }
        *data = out;
        break;

    case IT_CONT_DISABLE_BASIC_IRQ:
        for (int i = 0; i < 8; i++) {
            out |= !m_it_ap_enable_reg[i] << i;
        }
        *data = out;
        break;

    case IT_CONT_IRQ_BASIC_PENDING:
        pending_in_r1 = 0;
        pending_in_r2 = 0;

        for (int i = 0; i < 64; i++) {
            if (m_it_pending_reg[i]) {
                if (i < 32) {
                    pending_in_r1 = 1;
                } else {
                    pending_in_r2 = 1;
                }
            }
        }

        out = m_it_pending_reg[62] << 20 | m_it_pending_reg[57] << 19
                | m_it_pending_reg[56] << 18 | m_it_pending_reg[55] << 17
                | m_it_pending_reg[54] << 16 | m_it_pending_reg[53] << 15
                | m_it_pending_reg[19] << 14 | m_it_pending_reg[18] << 13
                | m_it_pending_reg[10] << 12 | m_it_pending_reg[9] << 11
                | m_it_pending_reg[7] << 10 | pending_in_r2 << 9
                | pending_in_r1 << 8;

        /* out[7:0] = m_it_pending[7:0] */
        for (int i = 0; i < IT_CONT_AP_PORT_NB; i++) {
            out |= m_it_ap_pending_reg[i] << i;
        }

        DPRINTF("BASIC_PENDING: %" PRIx32 " \n", out);
        *data = out;

        break;

    case IT_CONT_IRQ_PENDING_1:
        for (int i = 0; i < 32; i++) {
            out |= (m_it_pending_reg[i] << i);
        }
        DPRINTF("IRQ_PENDING_1: 0x%" PRIx32 " \n", out);
        *data = out;
        break;

    case IT_CONT_IRQ_PENDING_2:
        for (int i = 0; i < 32; i++) {
            out |= (m_it_pending_reg[i + 32] << i);
        }
        DPRINTF("IRQ_PENDING_2: 0x%" PRIx32 " \n", out);
        *data = out;
        break;

    default:
        printf("Bad %s::%s ofs=0x%" PRIx64 " !\n", name(), __FUNCTION__, ofs);
        exit(1);
        break;
    }
    bErr = false;
}

void raspberry_it_controller::it_thread()
{
    bool irq_pending;
    while (1) {
        wait(m_event_list);

        irq_pending = false;
        /* Checking GPU peripherials */
        for (int i = 0; i < m_it_ports; i++) {
            if (m_it_enable_reg[irq_num[i]] == 1) {
                if (irq_in[i] == 1) {
                    DPRINTF("IRQ(GPU) found: no %d\n", irq_num[i]);
                    irq_pending = true;
                    m_it_pending_reg[irq_num[i]] = 1;
                } else {
                    m_it_pending_reg[irq_num[i]] = 0;
                }
            }
        }
        /* Checking arm peripherials IRQ */
        for (int i = 0; i < m_it_ap_ports; i++) {
            if (m_it_ap_enable_reg[irq_ap_num[i]] == 1) {
                if (irq_ap_in[i] == 1) {
                    DPRINTF("IRQ(AP) found: no %d\n", irq_ap_num[i]);
                    irq_pending = true;
                    m_it_ap_pending_reg[irq_ap_num[i]] = 1;
                } else {
                    m_it_ap_pending_reg[irq_ap_num[i]] = 0;
                }
            }
        }

        if (irq_pending) {
            irq = true;
            DPRINTF("Rising IRQ on CPU\n");
        } else {
            irq = false;
        }
    }
}

void raspberry_it_controller::reset_registers(void)
{
    for (int i = 0; i < IT_CONT_PORT_NB; i++) {
        m_it_enable_reg[i] = 0;
        m_it_pending_reg[i] = 0;
    }
    for (int i = 0; i < IT_CONT_AP_PORT_NB; i++) {
        m_it_ap_enable_reg[i] = 0;
        m_it_pending_reg[i] = 0;
    }
}

