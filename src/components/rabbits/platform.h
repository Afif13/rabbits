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

#ifndef __GENERIC_SUBSYSTEM_H__
#define __GENERIC_SUBSYSTEM_H__

class Platform;

#include "components/rabbits/master.h"
#include "components/rabbits/slave.h"
#include "components/rabbits/interconnect.h"

#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <vector>

class Platform: public sc_module
{
protected:
    struct mapping_descr {
        uint64_t addr;
        uint64_t len;
    };

    std::vector<Slave *> m_targets;
    std::vector<Master *> m_initiators;
    std::vector<mapping_descr *> m_mappings;
    Interconnect<32> *m_interco;

    virtual void end_of_elaboration();

public:
    Platform(sc_module_name name);
    ~Platform();

    void connect_target(Slave *target, uint64_t addr, uint64_t len);
    
    template <unsigned int BUSWIDTH>
    void connect_target(tlm::tlm_target_socket<BUSWIDTH> *target, uint64_t addr, uint64_t len) {
        mapping_descr *descr = new mapping_descr;

        descr->addr = addr;
        descr->len = len;
        m_mappings.push_back(descr);

        m_interco->connect_target(target, addr, len);
    }

    void connect_initiator(Master *initiator);

    template <unsigned int BUSWIDTH>
    void connect_initiator(tlm::tlm_initiator_socket<BUSWIDTH> *initiator) {
        m_interco->connect_initiator(initiator);
    }

};

#endif /* __GENERIC_SUBSYSTEM_H__ */
