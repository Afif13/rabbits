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

#include "rabbits-common.h"
#include "slave.h"

#define MODNAME "slave"
#include "utils/utils.h"

Slave::Slave(sc_module_name module_name)
	: sc_module(module_name)
{
	m_node_id = -1;

    socket.bind(*this);
}

Slave::~Slave()
{
}

void Slave::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
	bool bErr = false;

	uint64_t addr = trans.get_address();
	uint8_t *buf = reinterpret_cast<uint8_t *>(trans.get_data_ptr());
	uint64_t size = trans.get_data_length();

	switch (trans.get_command()) {
	case tlm::TLM_WRITE_COMMAND:
		bus_cb_write(addr, buf, size, bErr);
		break;
	case tlm::TLM_READ_COMMAND:
		bus_cb_read(addr, buf, size, bErr);
		break;
	default:
		EPRINTF("Unknown command\n");
        abort();
	}

	// returning synchronous response
	trans.set_response_status(
			bErr ? tlm::TLM_GENERIC_ERROR_RESPONSE : tlm::TLM_OK_RESPONSE);
}

unsigned int Slave::transport_dbg(tlm::tlm_generic_payload& trans)
{
	uint64_t addr = trans.get_address();
	uint8_t *buf = reinterpret_cast<uint8_t *>(trans.get_data_ptr());
	uint64_t size = trans.get_data_length();

	switch (trans.get_command()) {
	case tlm::TLM_READ_COMMAND:
		return debug_read(addr, buf, size);
	case tlm::TLM_WRITE_COMMAND:
		return debug_write(addr, buf, size);
	default:
		EPRINTF("Unsupported transport debug command type\n");
		exit(1);
	}
}
