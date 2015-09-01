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


#ifndef _INTERCONNECT_NOC_H
#define _INTERCONNECT_NOC_H

template <unsigned int BUSWIDTH> class Interconnect_NoC;

#include <vector>

#include "rabbits-common.h"
#include "components/rabbits/interconnect.h"
#include "components/rabbits/node.h"
#include <tlm>

#define MODNAME "interconnect_NoC"
#include "utils/utils.h"


enum {
        ABSTRACT_NOC = 0,
        MESH = 1,
        TORUS = 2,
};


template <unsigned int BUSWIDTH = 32>
class Interconnect_NoC: public Interconnect<BUSWIDTH>
{
protected:
    struct RouteInfo {
	int x;
	int y;
	int z;
    };

    std::vector<node<BUSWIDTH> *> m_nodes;
    std::vector<RouteInfo *> route_table;
    int col;
    int row;
    int Z;

    int n_type;

public:
    tlm::tlm_initiator_socket<BUSWIDTH> * get_slave_target(int i) {
	return m_nodes[route_table[i]->z*col*row+route_table[i]->x*col+route_table[i]->y]->target_slave;
    }
    int get_route_x(int i){return route_table[i]->x;}
    int get_route_y(int i){return route_table[i]->y;}
    int get_route_z(int i){return route_table[i]->z;}

    /*  */

    int decode_address_i(sc_dt::uint64 addr, sc_dt::uint64& addr_offset)
    {
        unsigned int i;
        typename Interconnect<BUSWIDTH>::AddressRange *range;

        for (i = 0; i < Interconnect<BUSWIDTH>::m_ranges.size(); i++) {
            range = Interconnect<BUSWIDTH>::m_ranges[i];
            if (addr >= range->begin && addr < range->end)
                break;
        }
        if (i == Interconnect<BUSWIDTH>::m_ranges.size()) {
            return -1;
        }

        addr_offset = range->begin;
        return i;
    }

    SC_HAS_PROCESS(Interconnect_NoC);
    Interconnect_NoC(sc_module_name name) : Interconnect<BUSWIDTH>(name) {;}

    virtual ~Interconnect_NoC()
    {
	int i;

        for (i = 0; i < m_nodes.size(); i++) {
           delete m_nodes[i];
       }
    }


    void connect_initiator(tlm::tlm_initiator_socket<BUSWIDTH> *initiator,int a,int b,int c);

    void connect_target(tlm::tlm_target_socket<BUSWIDTH> *target,uint64_t addr, uint64_t len,int a,int b,int c);

    void create_network(int type){n_type=type;printf("\n---------\n Abstract NoC Created \n---------\n");} //the abstract NoC

    void create_network(int type,int a,int b,int h);


};

#undef MODNAME

#endif
