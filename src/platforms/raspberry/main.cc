/*
 *  Copyright (c) 2010 TIMA Laboratory
 *
 *  This file is part of Rabbits.
 *
 *  Rabbits is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Rabbits is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Rabbits.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <fcntl.h>

#include <systemc.h>

#include "system_init.h"
#include "raspberry.h"

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
        .gdb_port = 0
    };

    simu_manager simu;

    parse_cmdline(argc, argv, &config);
    if (check_init(&config) != 0) {
        fprintf(stderr, "Cannot parse command line arguments, exiting\n");
        exit(1);
    }

    Raspberry fram("raspberry", &config);

    ui::start_ui();

    simu.start();
    return 0;
}
