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

#ifndef _INTERCONNECT_H
#define _INTERCONNECT_H

#include <vector>

#include "rabbits-common.h"

#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

class slave_device;
class master_device;

class interconnect: public sc_module
{
public:
    struct AddressRange {
        tlm_utils::simple_initiator_socket<interconnect> * socket;
        uint64_t begin;
        uint64_t end;
    };

    SC_HAS_PROCESS(interconnect);
    interconnect(sc_module_name name);
    ~interconnect();

    template <class T_INITIATOR>
    void connect_initiator(tlm_utils::simple_initiator_socket<T_INITIATOR> *initiator)
    {
        tlm_utils::simple_target_socket<interconnect> *socket = new tlm_utils::simple_target_socket<interconnect>;
        socket->register_b_transport(this, &interconnect::b_transport);
        socket->register_transport_dbg(this, &interconnect::transport_dbg);

        m_initiators.push_back(socket);
        initiator->bind(*socket);
    }


    template <class T_TARGET>
    void connect_target(tlm_utils::simple_target_socket<T_TARGET> *target,
                                      uint64_t addr, uint64_t len)
    {
        tlm_utils::simple_initiator_socket<interconnect> *socket = new tlm_utils::simple_initiator_socket<interconnect>;
        socket->bind(*target);

        m_targets.push_back(socket);

        AddressRange *range = new AddressRange();
        range->socket = socket;
        range->begin = addr;
        range->end = addr + len;
        m_ranges.push_back(range);
    }

    tlm_utils::simple_initiator_socket<interconnect> *
    decode_address(sc_dt::uint64 addr, sc_dt::uint64& target_address);


protected:
    std::vector<tlm_utils::simple_target_socket<interconnect>  *> m_initiators;
    std::vector<tlm_utils::simple_initiator_socket<interconnect> *> m_targets;

    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans);

public:
    std::vector<AddressRange *> m_ranges;
};

#endif
