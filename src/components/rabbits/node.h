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


#ifndef _NODE_H
#define _NODE_H

template <unsigned int BUSWIDTH> class node;


#include "rabbits-common.h"
#include "components/rabbits/interconnect_noc.h"
#include <tlm_utils/simple_target_socket.h>

class Master;
struct route_info:tlm::tlm_extension<route_info> {

	route_info():x(0),y(0),z(0){}

	virtual tlm_extension_base* clone() const {
		route_info* t=new route_info;
		t->x=this->x;
		t->y=this->y;
		t->z=this->z;
		return t;
	}
	virtual void copy_from(tlm_extension_base const &ext) {
		x = static_cast<route_info const&>(ext).x;
		y = static_cast<route_info const&>(ext).y;
		z = static_cast<route_info const&>(ext).z;
	}

	int x;
	int y;
	int z;
};

template <unsigned int BUSWIDTH = 32>
class node: public sc_module,public tlm::tlm_fw_transport_if<>,public tlm::tlm_bw_transport_if<>
{
public:

    SC_HAS_PROCESS(node);
    node(sc_module_name name,Interconnect_NoC<BUSWIDTH> *p,int x,int y,int z);
    ~node();

    tlm::tlm_target_socket<BUSWIDTH> *get_top(void){return m_targets[0]; };
    tlm::tlm_target_socket<BUSWIDTH> *get_right(void){return m_targets[1]; };
    tlm::tlm_target_socket<BUSWIDTH> *get_bottom(void){return m_targets[2]; };
    tlm::tlm_target_socket<BUSWIDTH> *get_left(void){return m_targets[3]; };

    tlm::tlm_target_socket<BUSWIDTH> *get_up(void){return m_target_up; }; // need to call init_up() before
    tlm::tlm_target_socket<BUSWIDTH> *get_down(void){return m_target_down; }; // need to call init_down() before


    void connect_top(node *a){
	if(a->get_bottom())
		m_initiators[0]->bind(*(a->get_bottom()));

    };
    void connect_right(node *a){
	if(a->get_left())
		m_initiators[1]->bind(*(a->get_left()));

    };
    void connect_bottom(node *a){
	if(a->get_top())
		m_initiators[2]->bind(*(a->get_top()));

    };
    void connect_left(node *a){
	if(a->get_right())
		m_initiators[3]->bind(*(a->get_right()));

    };

    void init_up(void);
    void init_down(void);

    void connect_up(node *a){
	if(a->get_down())
		m_initiator_up->bind(*(a->get_down()));

    };

    void connect_down(node *a){
	if(a->get_up())
		m_initiator_down->bind(*(a->get_up()));

    };


    int get_x(void){return x;};
    int get_y(void){return y;};
    int get_z(void){return z;};
    int get_connect(void){return connect;};

    void set_x(int a){x=a;};
    void set_y(int a){y=a;};
    void set_z(int a){z=a;};
    void set_connect(int a){connect=a;};
protected:
    tlm::tlm_target_socket<BUSWIDTH>  *m_targets[4]; //0:Top - 1:Right - 2:Bottom - 3:Left
    tlm::tlm_initiator_socket<BUSWIDTH> *m_initiators[4]; //0:Top - 1:Right - 2:Bottom - 3:Left

    tlm::tlm_target_socket<BUSWIDTH> *m_target_up,*m_target_down;
    tlm::tlm_initiator_socket<BUSWIDTH> *m_initiator_up,*m_initiator_down;

public:
    //to connect with a potential slave
    tlm::tlm_initiator_socket<BUSWIDTH> *target_slave;
    //to connect with a potential master
    tlm_utils::simple_target_socket<node<BUSWIDTH>,BUSWIDTH> *initiator_master;
    void register_transport_master();

protected:
    void b_transport_master(tlm::tlm_generic_payload& trans, sc_time& delay);
    void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

    unsigned int transport_dbg(tlm::tlm_generic_payload& trans);
    unsigned int transport_dbg_master(tlm::tlm_generic_payload& trans);

    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,tlm::tlm_dmi& dmi_data);
    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,tlm::tlm_phase& phase,sc_core::sc_time& t){;};

    virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,tlm::tlm_phase& phase,sc_core::sc_time& t){;};
    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,sc_dt::uint64 end_range){;};

   int x;
   int y;
   int z;

   //to test if the node is connected to an IP or not
   int connect;

   //to use the Decode function and the Route Table
   Interconnect_NoC<BUSWIDTH> *p_inter;
};

#endif
