#ifndef __PLATFORM_NOC_H__
#define __PLATFORM_NOC_H__

class PlatformNoc;

#include "components/rabbits/master.h"
#include "components/rabbits/slave.h"
#include "components/rabbits/interconnect_noc_impl.h"
#include "components/rabbits/node_impl.h"

#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <vector>

class PlatformNoc: public sc_module
{
protected:
    struct mapping_descr {
        uint64_t addr;
        uint64_t len;
    };

    std::vector<Slave *> m_targets;
    std::vector<Master *> m_initiators;
    std::vector<mapping_descr *> m_mappings;

    InterconnectNoc<32> *m_interco;

    virtual void end_of_elaboration();

public:
    PlatformNoc(sc_module_name name);
    ~PlatformNoc();

    void connect_target(Slave *target, uint64_t addr, uint64_t len,
                            int x,int y,int z);

    void connect_target(Slave *target, uint64_t addr, uint64_t len,
                            int x,int y)
    {
        connect_target(target,addr,len,x,y,0);
    }

    void connect_target(Slave *target, uint64_t addr, uint64_t len);

    template <unsigned int BUSWIDTH>
    void connect_target(tlm::tlm_target_socket<BUSWIDTH> *target, uint64_t addr,
                            uint64_t len,int x,int y,int z)
    {
        mapping_descr *descr = new mapping_descr;

        descr->addr = addr;
        descr->len = len;
        m_mappings.push_back(descr);

        m_interco->connect_target(target, addr, len,x,y,z);
    }

    template <unsigned int BUSWIDTH>
    void connect_target(tlm::tlm_target_socket<BUSWIDTH> *target,
                            uint64_t addr, uint64_t len,int x,int y)
    {
        mapping_descr *descr = new mapping_descr;

        descr->addr = addr;
        descr->len = len;
        m_mappings.push_back(descr);

        m_interco->connect_target(target, addr, len,x,y,0);
    }

    template <unsigned int BUSWIDTH>
    void connect_target(tlm::tlm_target_socket<BUSWIDTH> *target,
                            uint64_t addr, uint64_t len)
    {
        mapping_descr *descr = new mapping_descr;

        descr->addr = addr;
        descr->len = len;
        m_mappings.push_back(descr);

        m_interco->Interconnect<BUSWIDTH>::connect_target(target, addr, len);
    }

    void connect_initiator(Master *initiator,int x,int y,int z);

    void connect_initiator(Master *initiator,int x,int y)
    {
        connect_initiator(initiator,x,y,0);
    }

    void connect_initiator(Master *initiator);

    template <unsigned int BUSWIDTH>
    void connect_initiator(tlm::tlm_initiator_socket<BUSWIDTH> *initiator,
                                int x, int y, int z)
    {
        m_interco->connect_initiator(initiator,x,y,z);
    }

    template <unsigned int BUSWIDTH>
    void connect_initiator(tlm::tlm_initiator_socket<BUSWIDTH> *initiator,
                                int x, int y)
    {
        m_interco->connect_initiator(initiator,x,y,0);
    }

    template <unsigned int BUSWIDTH>
    void connect_initiator(tlm::tlm_initiator_socket<BUSWIDTH> *initiator)
    {
        m_interco->Interconnect<BUSWIDTH>::connect_initiator(initiator);
    }

   void create_network(int type,int col,int row,int h) {
      m_interco->create_network(type,col,row,h);
   }

   void create_network(int type,int col,int row) {
      m_interco->create_network(type,col,row,1);
   }

   void create_network(int type) {
      m_interco->create_network(type);
   }

};

#endif /* __PLATFORM_NOC_H__ */
