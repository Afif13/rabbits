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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>

#include "system_init.h"

enum
{
    CMDLINE_OPTION_kernel,
    CMDLINE_OPTION_initrd,
    CMDLINE_OPTION_dtb,
    CMDLINE_OPTION_gdb_port,
    CMDLINE_OPTION_kernel_cmd,
    CMDLINE_OPTION_time,
};

typedef struct
{
    const char *name;
    int flags;
    int index;
} cmdline_option;

#define HAS_ARG             (1 << 0)

const cmdline_option cmdline_options[] = {
    { "kernel", HAS_ARG, CMDLINE_OPTION_kernel },
    { "initrd", HAS_ARG, CMDLINE_OPTION_initrd },
    { "dtb", HAS_ARG, CMDLINE_OPTION_dtb },
    { "gdb_port", HAS_ARG, CMDLINE_OPTION_gdb_port },
    { "time", HAS_ARG, CMDLINE_OPTION_time },
    { "append", HAS_ARG, CMDLINE_OPTION_kernel_cmd },
    { NULL, 0, 0 },
};

void parse_cmdline(int argc, char **argv, init_struct *is)
{
    int optind;
    const char *r, *optarg;

    optind = 1;
    for (;;) {
        if (optind >= argc)
            break;
        r = argv[optind];

        const cmdline_option *popt;

        optind++;
        /* Treat --foo the same as -foo.  */
        if (r[1] == '-')
            r++;
        popt = cmdline_options;
        for (;;) {
            if (!popt->name) {
                fprintf(stderr, "%s: invalid option -- '%s'\n", argv[0], r);
                exit(1);
            }
            if (!strcmp(popt->name, r + 1))
                break;
            popt++;
        }
        if (popt->flags & HAS_ARG) {
            if (optind >= argc) {
                fprintf(stderr, "%s: option '%s' requires an argument\n",
                        argv[0], r);
                exit(1);
            }
            optarg = argv[optind++];
        } else
            optarg = NULL;

        switch (popt->index) {
        case CMDLINE_OPTION_kernel:
            is->kernel_filename = optarg;
            break;
        case CMDLINE_OPTION_initrd:
            is->initrd_filename = optarg;
            break;
        case CMDLINE_OPTION_dtb:
            is->dtb_filename = optarg;
            break;
        case CMDLINE_OPTION_gdb_port:
            is->gdb_port = atoi(optarg);
            break;
        case CMDLINE_OPTION_time:
            is->time = atoi(optarg);
            break;
        case CMDLINE_OPTION_kernel_cmd:
            is->kernel_cmdline = optarg;
            break;
        }
    }
}

int check_init(init_struct *is)
{
    int error = 0;
    struct stat s;
    if (is->time == 1000) {
        printf("you didn't set a time (use -time option)\nDefault simulation time will be used\n");
    }
    return error;
}
