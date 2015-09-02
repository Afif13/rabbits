#include "platform_noc.h"

#define MODNAME "PlatformNoc"
#include "utils/utils.h"

PlatformNoc::PlatformNoc(sc_module_name name) :
        sc_module(name)
{
    m_interco = new InterconnectNoc<32>("interconnect");
}

PlatformNoc::~PlatformNoc()
{
    delete m_interco;
}

void PlatformNoc::connect_target(Slave *target,
                                    uint64_t addr, uint64_t len,
                                    int x, int y, int z)
{
    m_targets.push_back(target);
    connect_target(&target->socket, addr, len,x,y,z);
}

void PlatformNoc::connect_target(Slave *target,
                                    uint64_t addr, uint64_t len)
{
    m_targets.push_back(target);
    connect_target(&target->socket, addr, len);
}

void PlatformNoc::connect_initiator(Master *initiator,
                                    int x, int y, int z)
{
    m_initiators.push_back(initiator);
    connect_initiator(&initiator->socket,x,y,z);
}

void PlatformNoc::connect_initiator(Master *initiator) {
    m_initiators.push_back(initiator);
    connect_initiator(&initiator->socket);
}

void PlatformNoc::end_of_elaboration()
{
    std::vector<Master *>::iterator master;
    std::vector<mapping_descr *>::iterator descr;

    for (master = m_initiators.begin(); master != m_initiators.end(); master++) {
        for (descr = m_mappings.begin(); descr != m_mappings.end(); descr++) {
            (*master)->dmi_hint((*descr)->addr, (*descr)->len);
        }
    }
}
