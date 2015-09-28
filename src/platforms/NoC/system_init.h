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
