#include "platform_noc.h"

#define MODNAME "Platform_NoC"
#include "utils/utils.h"

Platform_NoC::Platform_NoC(sc_module_name name) :
        sc_module(name)
{
    m_interco = new Interconnect_NoC<32>("interconnect");
}

Platform_NoC::~Platform_NoC()
{
    delete m_interco;
}

void Platform_NoC::connect_target(Slave *target, uint64_t addr, uint64_t len,int x,int y,int z) {
    m_targets.push_back(target);
    connect_target(&target->socket, addr, len,x,y,z);
}

void Platform_NoC::connect_target(Slave *target, uint64_t addr, uint64_t len) {
    m_targets.push_back(target);
    connect_target(&target->socket, addr, len);
}

void Platform_NoC::connect_initiator(Master *initiator,int x,int y,int z) {
    m_initiators.push_back(initiator);
    connect_initiator(&initiator->socket,x,y,z);
}

void Platform_NoC::connect_initiator(Master *initiator) {
    m_initiators.push_back(initiator);
    connect_initiator(&initiator->socket);
}

void Platform_NoC::end_of_elaboration() {
    std::vector<Master *>::iterator master;
    std::vector<mapping_descr *>::iterator descr;

    for (master = m_initiators.begin(); master != m_initiators.end(); master++) {
        for (descr = m_mappings.begin(); descr != m_mappings.end(); descr++) {
            (*master)->dmi_hint((*descr)->addr, (*descr)->len);
        }
    }
}
