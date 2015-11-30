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

#include "components/raspberry/uart/raspberry_uart.h"
#include "components/raspberry/system_timer/raspberry_system_timer.h"
#include "components/raspberry/gpio/raspberry_gpio.h"
#include "components/raspberry/timer/raspberry_arm_timer.h"
#include "components/raspberry/it_ctrl/raspberry_it_controller.h"
#include "components/raspberry/vcore/mailbox.h"
#include "components/raspberry/vcore/vcore.h"
#include "components/raspberry/dma/dma.h"

#include "components/rabbits/platform.h"

#include "system_init.h"

class Raspberry: public Platform
{
private:
    /*
     * Components of the Raspberry Platform
     */
    memory *m_ram;
    qemu_wrapper<qemu_cpu_arm> *m_qemu;
    raspberry_uart *m_tty;
    raspberry_system_timer *m_systimer;
    raspberry_it_controller *m_it_cont;
    raspberry_gpio *m_gpio;
    raspberry_arm_timer *m_armtimer;
    rpi_vcore *m_vcore;
    rpi_dma *m_dma;

    DebugInitiator *m_dbg_initiator;

    /*
     * ARM IRQ wiring
     */
    sc_signal<bool> m_irq_line;

    void end_of_elaboration();
    void load_bootloader(init_struct*);

public:
    Raspberry(sc_module_name _name, init_struct *config);

    unsigned char *systemc_get_sram_mem_addr(void);
    void set_kernel_args(uint32_t ram_size, uint32_t initrd_size,
            const char *kernel_cmdline, uint32_t loader_start);
};
