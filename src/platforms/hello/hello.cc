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

#include "hello.h"
#include "utils/bootloader.h"

#define MODNAME "hello"
#define DEBUG_TRACE
#include "utils/utils.h"

#define RAMSIZE 1024

Hello::Hello(sc_module_name _name) :
        Platform(_name)
{
    m_qemu = new qemu_wrapper<qemu_cpu_arm>("QEMU", 1, "arm1176");

    //slaves
    m_ram = new memory("dynamic", RAMSIZE);
    m_dbg_initiator = new DebugInitiator("dbg-initiator");

    connect_target(m_ram, 0x00000000ul, RAMSIZE);

    //masters

    connect_initiator(m_qemu);
    connect_initiator(&m_dbg_initiator->socket);

    load_helloworld();
}

void Hello::load_helloworld()
{
    uint8_t helloworld[] = {
        // Interrupt Vector Table
        0x06, 0x00, 0x00, 0xea, // reset
        0xfe, 0xff, 0xff, 0xea, // undefined instruction
        0xfe, 0xff, 0xff, 0xea, // SWI
        0xfe, 0xff, 0xff, 0xea, // prefetch abort
        0xfe, 0xff, 0xff, 0xea, // data abort
        0xfe, 0xff, 0xff, 0xea, // reserved
        0xfe, 0xff, 0xff, 0xea, // IRQ
        0xfe, 0xff, 0xff, 0xea, // FIQ

        // Hello, World!
        0x01, 0x12, 0xa0, 0xe3,
        0x48, 0x20, 0xa0, 0xe3,
        0x65, 0x30, 0xa0, 0xe3,
        0x6c, 0x40, 0xa0, 0xe3,
        0x6f, 0x50, 0xa0, 0xe3,
        0x2c, 0x60, 0xa0, 0xe3,
        0x20, 0x70, 0xa0, 0xe3,
        0x57, 0x80, 0xa0, 0xe3,
        0x72, 0x90, 0xa0, 0xe3,
        0x64, 0xa0, 0xa0, 0xe3,
        0x21, 0xb0, 0xa0, 0xe3,
        0x00, 0x20, 0x81, 0xe5,
        0x00, 0x30, 0x81, 0xe5,
        0x00, 0x40, 0x81, 0xe5,
        0x00, 0x40, 0x81, 0xe5,
        0x00, 0x50, 0x81, 0xe5,
        0x00, 0x60, 0x81, 0xe5,
        0x00, 0x70, 0x81, 0xe5,
        0x00, 0x80, 0x81, 0xe5,
        0x00, 0x50, 0x81, 0xe5,
        0x00, 0x90, 0x81, 0xe5,
        0x00, 0x40, 0x81, 0xe5,
        0x00, 0xa0, 0x81, 0xe5,
        0x00, 0xb0, 0x81, 0xe5,

        // Endless Loop
        0xfe, 0xff, 0xff, 0xea,
    };

    m_dbg_initiator->debug_write(0, helloworld, sizeof(helloworld));
}


void Hello::end_of_elaboration() 
{
    Platform::end_of_elaboration();
}
