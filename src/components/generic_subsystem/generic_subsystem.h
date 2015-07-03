#ifndef __GENERIC_SUBSYSTEM_H__
#define __GENERIC_SUBSYSTEM_H__

class generic_subsystem;
typedef generic_subsystem generic_subsystem_t;

#include "components/master_device/master_device.h"
#include "components/slave_device/slave_device.h"
#include "components/interconnect/interconnect.h"
#include "components/qemu_wrapper/qemu_wrapper.h"

#include <tlm_utils/simple_initiator_socket.h>
#include <vector>

class generic_subsystem: public sc_module
{

public:
    std::vector<slave_device *> m_targets;
    std::vector<master_device *> m_initiators;
    interconnect *m_onoc;

public:
    generic_subsystem(sc_module_name name);
    ~generic_subsystem();

    void connect_target(slave_device *target, uint64_t addr, uint64_t len);
    
    template <class T_TARGET>
    void connect_target(tlm_utils::simple_target_socket<T_TARGET> *target) {
        m_onoc->connect_target<T_TARGET>(target);
    }


    void connect_initiator(master_device *initiator);

    template <class T_INITIATOR>
    void connect_initiator(tlm_utils::simple_initiator_socket<T_INITIATOR> *initiator) {
        m_onoc->connect_initiator<T_INITIATOR>(initiator);
    }

};

#endif /* __GENERIC_SUBSYSTEM_H__ */
