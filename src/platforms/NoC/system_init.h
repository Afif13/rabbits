/*
 *  Copyright (c) 2015 Clement Deschamps and Luc Michel
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

#ifndef _SYSTEM_INIT_H_
#define _SYSTEM_INIT_H_

#include <stdint.h>

typedef struct
{
    const char *kernel_filename;
    const char *dtb_filename;
    const char *initrd_filename;
    const char *kernel_cmdline;
    int 	gdb_port;
    int 	time;
} init_struct;

extern void parse_cmdline(int argc, char **argv, init_struct *is);
extern int check_init(init_struct *is);
extern void arm_load_kernel(init_struct *is);

#endif
