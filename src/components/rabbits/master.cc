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

#include "master.h"

#define MODNAME "master"
#include "utils/utils.h"

Master::Master(sc_module_name module_name) :
        sc_module(module_name)
{
    m_node_id = -1;

}

Master::~Master(void)
{
}

void Master::bus_access(tlm::tlm_command cmd, uint64_t addr, uint8_t *data,
                               unsigned int len)
{
    DPRINTF("rw: addr=%p, data=%p, len=%d\n", (void *) addr, data,
            len);

    if (!data) {
        EPRINTF("bus_access: error, data is null\n");
        exit(1);
    }

    tlm::tlm_generic_payload trans;
    sc_time delay = SC_ZERO_TIME;

    trans.set_command(cmd);
    trans.set_address(addr);
    trans.set_data_ptr(data);
    trans.set_data_length(len);
    trans.set_streaming_width(len);
    trans.set_byte_enable_ptr(NULL);
    trans.set_byte_enable_length(0);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_dmi_allowed(false);
    socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("Master", "response error from b_transport");
    }
}

void Master::bus_read(uint64_t addr, uint8_t *data, unsigned int len)
{
    bus_access(tlm::TLM_READ_COMMAND, addr, data, len);
}

void Master::bus_write(uint64_t addr, uint8_t *data, unsigned int len)
{
    bus_access(tlm::TLM_WRITE_COMMAND, addr, data, len);
}

void Master::dmi_hint(uint64_t start, uint64_t size)
{
    tlm::tlm_generic_payload trans;
    tlm::tlm_dmi dmi_data;

    trans.set_address(static_cast<sc_dt::uint64>(start));
    trans.set_command(tlm::TLM_READ_COMMAND);

    if (socket->get_direct_mem_ptr(trans, dmi_data)) {
        if (dmi_data.is_read_write_allowed() 
            && dmi_data.get_start_address() == start 
            && dmi_data.get_end_address() == start + size - 1) {
            /* DMI ok */
            dmi_hint_cb(start, size, static_cast<void*>(dmi_data.get_dmi_ptr()),
                        dmi_data.get_read_latency(), dmi_data.get_write_latency());
        } else {
            /* Not enough DMI requirements, ignore it */
            dmi_hint_cb(start, size, NULL, SC_ZERO_TIME, SC_ZERO_TIME);
        }
    } else {
        /* No DMI here */
        dmi_hint_cb(start, size, NULL, SC_ZERO_TIME, SC_ZERO_TIME);
    }
}
