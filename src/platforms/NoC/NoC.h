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

#include "components/rabbits/interconnect_noc.h"
#include "components/generator/generator.h"

#include "components/rabbits/platform_noc.h"

#include "system_init.h"

class NoC: public PlatformNoc
{
private:
    /*
     * Components of the NoC Platform
     */
    memory *m_ram[5];
    Generator *m_gen[5];

    void end_of_elaboration();

public:
    NoC(sc_module_name _name, init_struct *config);
    ~NoC();
    void set_kernel_args(uint32_t ram_size, uint32_t initrd_size,
            const char *kernel_cmdline, uint32_t loader_start);
};
