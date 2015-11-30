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
#include "components/rabbits/debug/debug_initiator.h"

DebugInitiator::DebugInitiator(sc_module_name n) : sc_module(n)
{
}

DebugInitiator::~DebugInitiator()
{
}


uint64_t DebugInitiator::debug_read(uint64_t addr, void *buf, uint64_t size)
{
    tlm::tlm_generic_payload pl;

    pl.set_address(addr);
    pl.set_read();
    pl.set_data_ptr(reinterpret_cast<unsigned char*>(buf));
    pl.set_data_length(size);

    return static_cast<uint64_t>(socket->transport_dbg(pl));
}

uint64_t DebugInitiator::debug_write(uint64_t addr, void *buf, uint64_t size)
{
    tlm::tlm_generic_payload pl;

    pl.set_address(addr);
    pl.set_write();
    pl.set_data_ptr(reinterpret_cast<unsigned char*>(buf));
    pl.set_data_length(size);

    return static_cast<uint64_t>(socket->transport_dbg(pl));
}
