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

#include "raspberry.h"
#include "utils/bootloader.h"

#define MODNAME "raspberry"
#define DEBUG_TRACE
#include "utils/utils.h"

#define MACHINE_ID          0x00000c42
#define RAMSIZE             (256 * 1024 * 1024)


Raspberry::Raspberry(sc_module_name _name, init_struct *cfg) :
        Platform(_name)
{
    m_qemu = new qemu_wrapper<qemu_cpu_arm>("QEMU", 1, "arm1176");

    //slaves
    m_ram = new memory("dynamic", RAMSIZE);
    m_tty = new raspberry_uart("tty");
    m_gpio = new raspberry_gpio("gpio");
    m_armtimer = new raspberry_arm_timer("armtimer");
    m_systimer = new raspberry_system_timer("systimer");
    m_vcore = new rpi_vcore("vcore");
    m_dma = new rpi_dma("rpi-dma");
    m_dbg_initiator = new DebugInitiator("dbg-initiator");

    connect_target(m_ram, 0x00000000ul, RAMSIZE);
    connect_target(m_tty, 0x20201000ul, 0x1000);
    connect_target(m_systimer, 0x20003000UL, 0x1c );
    connect_target(m_armtimer, 0x2000B400UL, 0x24);

    /* Current mapping:
     *
     *      SYSTEM TIMER -> GPU irq 3
     *      UART -> GPU irq 57 
     *      ARM TIMER -> AP irq 0 
     *      Mailbox -> AP irq 1
     *
     */
    sc_signal<bool> *it_map = new sc_signal<bool> [2];
    int it_numbers[2] = { 57, 3 };

    sc_signal<bool> *it_ap_map = new sc_signal<bool> [2];
    int it_ap_numbers[2] = { 0, 1 };

    m_armtimer->irq(it_ap_map[0]);
    m_tty->irq_line(it_map[0]);
    m_systimer->irq(it_map[1]);
    m_vcore->get_mbox()->irq_line(it_ap_map[1]);

    m_it_cont = new raspberry_it_controller("IT_CONT", 2, it_map, it_numbers, 2,
            it_ap_map, it_ap_numbers);
    connect_target(m_it_cont, 0x2000b200UL, 0x200);

    connect_target(m_gpio, 0x20200000UL, 0xB4);
    connect_target(m_vcore->get_mbox(), 0x2000b880UL, 0x24);
    connect_target(m_dma, 0x20007000UL, 0xff4);

    //masters

    /* Connecting ARM irq line to irq controller */
    m_it_cont->irq(m_irq_line);
    m_qemu->get_cpu(0)->get_line_irq()(m_irq_line);

    connect_initiator(m_qemu);
    connect_initiator(m_vcore);
    connect_initiator(&m_dbg_initiator->socket);

    load_bootloader(cfg);
}

void Raspberry::load_bootloader(init_struct *cfg)
{

    ArmBootloader bootloader(m_dbg_initiator);

    ArmBootloader::PatchBlob::Entry entry_array[] = {
        { 0xe3a00000, ArmBootloader::FIXUP_NONE }, /* mov     r0, #0 */
        { 0xe59f1004, ArmBootloader::FIXUP_NONE }, /* ldr     r1, [pc, #4] */
        { 0xe59f2004, ArmBootloader::FIXUP_NONE }, /* ldr     r2, [pc, #4] */
        { 0xe59ff004, ArmBootloader::FIXUP_NONE }, /* ldr     pc, [pc, #4] */
        { 0,          ArmBootloader::FIXUP_MACHINE_ID },
        { 0,          ArmBootloader::FIXUP_BOOT_DATA },
        { 0,          ArmBootloader::FIXUP_KERNEL_ENTRY }
    };

    ArmBootloader::PatchBlob entry(entry_array, ARRAY_SIZE(entry_array));


    bootloader.set_ram_start(0);
    bootloader.set_kernel_image(cfg->kernel_filename);
    bootloader.set_kernel_load_addr(0x8000);

    if (cfg->dtb_filename) {
        bootloader.set_dtb(cfg->dtb_filename);
        bootloader.set_dtb_load_addr(0x100);
        bootloader.set_machine_id(0xffffffff);
    } else {
        bootloader.set_machine_id(MACHINE_ID);
    }

    bootloader.set_entry_blob(entry);

    if(bootloader.boot()) {
        EPRINTF("Bootloading failed. Exiting\n");
        exit(1);
    }
}


void Raspberry::end_of_elaboration() 
{
    Platform::end_of_elaboration();
}
