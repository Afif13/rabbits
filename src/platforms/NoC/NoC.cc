#include "NoC.h"
#include "utils/bootloader.h"

#define MODNAME "NoC"
#define DEBUG_TRACE
#include "utils/utils.h"

#define RAMSIZE             (256 * 1024 * 1024)

/*
    HOW TO USE the NoC

    1) To use the Abstract NoC Model do these steps :
    - Call create_network(ABSTRACT_NOC)
        (this step isn't mandatory for the abstract NoC since there is no network)
    - Call connect_initiator(init) and connect_target(target)
        to perform the connection

    2) To use the 2D-NoC Model do these steps :
    - Call create_network(TYPE,row,col)
        TYPE can be MESH or TORUS and (row,col) are the dimension of the 2D-Grid
    - Call connect_initiator(init,x,y) and connect_target(target,x,y)
        to connect each component on the Grid

    3) To use the 3D-NoC model we do like in 2D-NoC and we add a third parametre:
    - create_network(TYPE,row,col,Z)
    - connect_initiator(init,x,y,z) and connect_target(target,x,y,z)


    In order to change the Routing Algo we need to modify the file node_impl.h located
    at "src/components/rabbits/" :

    #define ROUTING_3D z_first_routing/xy_first_routing
    #define ROUTING_2D x_first_routing/y_first_routing

*/


NoC::NoC(sc_module_name _name, init_struct *cfg) :
        PlatformNoc(_name)
{

    m_ram[0] = new memory("r0", RAMSIZE);
    m_ram[1] = new memory("r1", RAMSIZE);
    m_ram[2] = new memory("r2", RAMSIZE);
    m_ram[3] = new memory("r3", RAMSIZE);



    /* Generator will send random read/write request to the corresponding memory with a certain period (in SC_NS)
       Generator(name,start_address,size,period)
    */
    m_gen[0] = new Generator("gen0",0x00000000ul,RAMSIZE,1);
    m_gen[1] = new Generator("gen1",0x10000000ul,RAMSIZE,1);
    m_gen[2] = new Generator("gen2",0x20000000ul,RAMSIZE,1);
    m_gen[3] = new Generator("gen3",0x30000000ul,RAMSIZE,1);



    create_network(MESH,2,6);

    //Slaves
    connect_target(m_ram[0], 0x00000000ul, RAMSIZE,0,4);
    connect_target(m_ram[1], 0x10000000ul, RAMSIZE,0,3);
    connect_target(m_ram[2], 0x20000000ul, RAMSIZE,0,2);
    connect_target(m_ram[3], 0x30000000ul, RAMSIZE,0,1);

    //Masters
    connect_initiator(m_gen[0],1,0);
    connect_initiator(m_gen[1],1,1);
    connect_initiator(m_gen[2],1,2);
    connect_initiator(m_gen[3],1,3);

}


NoC::~NoC()
{
    DPRINTF("Simulation ended after t = %lu ns\n",sc_time_stamp().value()/1000);
}

void NoC::end_of_elaboration()
{

}
