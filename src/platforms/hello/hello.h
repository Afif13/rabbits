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

#include <components/memory/memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <systemc.h>

#include "components/rabbits/interconnect.h"
#include "components/rabbits/debug/debug_initiator.h"

#include "components/qemu_wrapper/qemu_wrapper.h"
#include "components/qemu_wrapper/cpu/arm.h"

#include "components/rabbits/platform.h"

class Hello: public Platform
{
private:
    /*
     * Components of the Raspberry Platform
     */
    memory *m_ram;
    qemu_wrapper<qemu_cpu_arm> *m_qemu;

    DebugInitiator *m_dbg_initiator;

    /*
     * ARM IRQ wiring
     */
    sc_signal<bool> m_irq_line;

    void end_of_elaboration();
    void load_helloworld();

public:
    Hello(sc_module_name _name);

};
