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

#include "platform.h"

#define MODNAME "platform"
#include "utils/utils.h"

Platform::Platform(sc_module_name name) :
        sc_module(name)
{
    m_interco = new Interconnect<32>("interconnect");
}

Platform::~Platform()
{
    delete m_interco;
}

void Platform::connect_target(Slave *target, uint64_t addr, uint64_t len) {
    m_targets.push_back(target);
    connect_target(&target->socket, addr, len);
}

void Platform::connect_initiator(Master *initiator) {
    m_initiators.push_back(initiator);
    connect_initiator(&initiator->socket);
}

void Platform::end_of_elaboration() {
    std::vector<Master *>::iterator master;
    std::vector<mapping_descr *>::iterator descr;

    for (master = m_initiators.begin(); master != m_initiators.end(); master++) {
        for (descr = m_mappings.begin(); descr != m_mappings.end(); descr++) {
            (*master)->dmi_hint((*descr)->addr, (*descr)->len);
        }
    }
}
