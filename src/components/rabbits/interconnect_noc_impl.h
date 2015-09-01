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

#include "interconnect_noc.h"

#define MODNAME "interconnect_NoC"
#include "utils/utils.h"


template <unsigned int BUSWIDTH>
void Interconnect_NoC<BUSWIDTH>::connect_initiator(tlm::tlm_initiator_socket<BUSWIDTH> *initiator,int a,int b,int c)
{
        if(a < 0 || a > col) {
                printf("Cannot Connect Initiator, x is out of the range\n");
                exit(1);
        }
        if(b < 0 || b > row) {
                printf("Cannot Connect Initiator, y is out of the range");
                exit(1);
        }
        if( c < 0 || c > Z) {
                printf("Cannot Connect Initiator, z is out of the range");
                exit(1);
        }

        node<BUSWIDTH> *n = m_nodes[c*col*row+a*col+b];
        if(n->get_connect() == 1) {
                printf("Node already connected");
                exit(1);
        }
        n->set_connect(1);
        n->initiator_master = new tlm_utils::simple_target_socket<node<BUSWIDTH>,BUSWIDTH>;
        n->register_transport_master();
        initiator->bind(*(n->initiator_master));
}

template <unsigned int BUSWIDTH>
void Interconnect_NoC<BUSWIDTH>::connect_target(tlm::tlm_target_socket<BUSWIDTH> *target,
                                      uint64_t addr, uint64_t len,int a,int b,int c)
    {
        if(a < 0 || a > col) {
                printf("Cannot Connect Target, x are out of the range");
                exit(1);
        }
        if(b < 0 || b > row) {
                printf("Cannot Connect Target, y are out of the range");
                exit(1);
        }
        if( c < 0 || c > Z) {
                printf("Cannot Connect Target, z is out of the range");
                exit(1);
        }

        node<BUSWIDTH> *n = m_nodes[c*col*row+a*col+b];
        if(n->get_connect() == 1) {
                printf("Node already connected");
                exit(1);
        }
        n->set_connect(1);
        n->target_slave = new tlm::tlm_initiator_socket<BUSWIDTH>;
        n->target_slave->bind(*n);
        n->target_slave->bind(*target);


        typename Interconnect<BUSWIDTH>::AddressRange *range = new typename Interconnect<BUSWIDTH>::AddressRange;
	RouteInfo *info = new RouteInfo();
        range->begin = addr;
        range->end = addr + len;
        info->x=a;
        info->y=b;
        info->z=c;
        Interconnect<BUSWIDTH>::m_ranges.push_back(range);
        route_table.push_back(info);
}

template <unsigned int BUSWIDTH>
void Interconnect_NoC<BUSWIDTH>::create_network(int type,int a,int b,int h) {
        int i,j,k;
        char ch[10];
        node<BUSWIDTH> *n;

        col=a;
        row=b;
        Z=h;
        n_type=type;
        //creating all the nodes
        for(k=0;k<Z;k++) {
                for(i=0;i<row;i++) {
                        for(j=0;j<col;j++) {
                                sprintf(ch,"N[%d,%d,%d]",i,j,k);
                                n = new node<BUSWIDTH>(ch,this,i,j,k);
                                m_nodes.push_back(n);
                        }
                }
        }

        //connceting all nodes in the 2D  !!
        for(k=0;k<Z;k++) {
                for(i=0;i<row-1;i++) {
                        for(j=0;j<col-1;j++) {
                                m_nodes[k*col*row+i*col+j]->connect_right(m_nodes[k*col*row+i*col+j+1]);
                                m_nodes[k*col*row+i*col+j+1]->connect_left(m_nodes[k*col*row+i*col+j]);
                                m_nodes[k*col*row+i*col+j]->connect_bottom(m_nodes[k*col*row+(i+1)*col+j]);
                                m_nodes[k*col*row+(i+1)*col+j]->connect_top(m_nodes[k*col*row+i*col+j]);
                        }
                        //the last node of the row doesn't have a right node
                        m_nodes[k*col*row+i*col+col-1]->connect_bottom(m_nodes[k*col*row+(i+1)*col+col-1]);
                        m_nodes[k*col*row+(i+1)*col+col-1]->connect_top(m_nodes[k*col*row+i*col+col-1]);
                }
                //the last row alone
                for(j=0;j<col-1;j++) {
                        m_nodes[k*col*row+(row-1)*col+j]->connect_right(m_nodes[k*col*row+(row-1)*col+j+1]);
                        m_nodes[k*col*row+(row-1)*col+j+1]->connect_left(m_nodes[k*col*row+(row-1)*col+j]);
                }

                //Connect the port used for the TORUS even not used in MESH (systemC doesn't allow unbinded port)
                for(j=0;j<col;j++) {
                        m_nodes[k*col*row+j]->connect_top(m_nodes[k*col*row+(row-1)*col+j]);
                        m_nodes[k*col*row+(row-1)*col+j]->connect_bottom(m_nodes[k*col*row+j]);
                }

                for(i=0;i<row;i++) {
                        m_nodes[k*col*row+i*col]->connect_left(m_nodes[k*col*row+i*col+col-1]);
                        m_nodes[k*col*row+i*col+col-1]->connect_right(m_nodes[k*col*row+i*col]);
                }
        }
        //Doing the Up/Down connection for the 3D
	for(k=0;k<Z-1;k++) {
		for(i=0;i<row;i++) {
			for(j=0;j<col;j++) {
				m_nodes[k*col*row+i*col+j]->init_up();
				m_nodes[(k+1)*col*row+i*col+j]->init_down();
				m_nodes[k*col*row+i*col+j]->connect_up(m_nodes[(k+1)*col*row+i*col+j]);
				m_nodes[(k+1)*col*row+i*col+j]->connect_down(m_nodes[k*col*row+i*col+j]);
			}
		}
	}

	//.....
        if(Z==1) {
                printf("\n------\n 2D NoC Created\n");
                if(n_type==1)printf("Topologie : MESH \n");
                else printf("Topologie : TORUS \n");
                printf("nb column = %d nb row = %d\n",col,row);
                printf("------\n");
        } else {
                printf("\n------\n 3D NoC Created\n");
                if(n_type==1)printf("Topologie : MESH \n");
                else printf("Topologie : TORUS \n");
                printf("nb column = %d nb row = %d Z = %d\n",col,row,Z);
                printf("------\n");
        }
}



#undef MODNAME
