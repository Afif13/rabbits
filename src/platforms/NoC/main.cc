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

#include <unistd.h>
#include <fcntl.h>

#include <systemc.h>

#include "system_init.h"
#include "NoC.h"

#include "ui/ui.h"
#include "utils/simu.h"

int sc_main(int argc, char **argv)
{
    init_struct
    config = {
        .kernel_filename = NULL,
        .dtb_filename = NULL,
        .initrd_filename = NULL,
        .kernel_cmdline = NULL,
        .gdb_port = 0,
	.time = 1000
    };

    simu_manager simu;

    parse_cmdline(argc, argv, &config);
    check_init(&config);

    NoC fram("NoC", &config);

    printf("Start Simulation with T = %lu ns\n",config.time);
    sc_start(config.time,SC_NS);
    return 0;
}
