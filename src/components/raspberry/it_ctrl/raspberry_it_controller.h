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

#ifndef _raspberry_it_controller_H_
#define _raspberry_it_controller_H_

#include "components/rabbits/slave.h"

/* number of GPU input interrupt port */
#define IT_CONT_PORT_NB 64
/* number of ARM peripherial interrupt port */
#define IT_CONT_AP_PORT_NB 8

#define IT_CONT_IRQ_BASIC_PENDING   0x0
#define IT_CONT_IRQ_PENDING_1       0x4
#define IT_CONT_IRQ_PENDING_2       0x8
#define IT_CONT_FIQ_CONTROL         0xC
#define IT_CONT_ENABLE_IRQ_1        0x10
#define IT_CONT_ENABLE_IRQ_2        0x14
#define IT_CONT_ENABLE_BASIC_IRQ    0x18
#define IT_CONT_DISABLE_IRQ_1       0x1C
#define IT_CONT_DISABLE_IRQ_2       0x20
#define IT_CONT_DISABLE_BASIC_IRQ   0x24

class raspberry_it_controller: public Slave
{
public:
    SC_HAS_PROCESS (raspberry_it_controller);
    raspberry_it_controller(sc_module_name module_name, int it_ports,
            sc_signal<bool>* irq_wires, int* irq_wires_num, int it_ap_ports,
            sc_signal<bool>* irq_ap_wires, int* irq_ap_wires_num);
    virtual ~raspberry_it_controller();

private:
    void bus_cb_read_32(uint64_t addr, uint32_t *data, bool &bErr);
    void bus_cb_write_32(uint64_t addr, uint32_t *data, bool &bErr);

    void it_thread(void);
    void it_check(void);

public:
    //ports
    sc_out<bool> irq;

    /* Variable number of input ports.
     * The array irq_num is used to get the corresponding number of irq_in[i]
     */
    sc_in<bool> *irq_in;
    sc_in<bool> *irq_ap_in;

private:
    void reset_registers(void);

    sc_event ev_wake;
    sc_event ev_refresh;
    sc_event_or_list m_event_list;

    /* Enable and pending registers for GPU interrupts */
    int m_it_ports;
    int *irq_num;
    int m_it_enable_reg[IT_CONT_PORT_NB];
    int m_it_pending_reg[IT_CONT_PORT_NB];

    /* Enable and pending registers for arm peripherials */
    int m_it_ap_ports;
    int *irq_ap_num;
    int m_it_ap_enable_reg[IT_CONT_AP_PORT_NB];
    int m_it_ap_pending_reg[IT_CONT_AP_PORT_NB];
};

#endif

/*
 * Vim standard variables
 * vim:set ts=4 expandtab tw=80 cindent syntax=c:
 *
 * Emacs standard variables
 * Local Variables:
 * mode: c
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
