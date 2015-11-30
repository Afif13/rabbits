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

#ifndef _QEMU_WRAPPER_H
#define _QEMU_WRAPPER_H

#include <vector>
#include <cassert>

#include "qemu_lib_wrapper.h"
#include "dev/qemu_device.h"
#include "components/rabbits/master.h"

#define MODNAME "qemu-wrapper"
#include "utils/utils.h"

template <class t_cpu>
class qemu_wrapper: public Master, public qemu_io_callbacks
{
protected:
    qemu_lib_wrapper m_lib;

    std::vector<t_cpu*> m_cpus;
    std::vector<qemu_device*> m_devices;

    virtual uint32_t qemu_io_read(uint32_t addr, uint32_t size) {
        uint32_t buf;

        assert(size <= sizeof(buf));
        bus_read(addr, (uint8_t*)&buf, size);

        return buf;
    }

    virtual void qemu_io_write(uint32_t addr, uint32_t val, uint32_t size) {
        assert(size <= sizeof(val));
        bus_write(addr, (uint8_t*)&val, size);
    }

    void cpus_thread() {
        bool qemu_want_quit = false;

        for(;;) {
            qemu_want_quit = m_lib.cpus_loop();

            if(qemu_want_quit) {
                sc_stop();
            }

            wait(100, SC_NS);
        }
    }

    virtual void dmi_hint_cb(uint64_t start, uint64_t size, void *data,
                             sc_time read_latency, sc_time write_latency)
    {
        if (data != NULL) {
            DPRINTF("DMI mapping from %" PRIx64 " to %" PRIx64 "\n", start, start+size);
            add_map_dmi(start, size, data);
        } else {
            DPRINTF("I/O mapping from %" PRIx64 " to %" PRIx64 "\n", start, start+size);
            add_map(start, size);
        }
    }

public:
    SC_HAS_PROCESS (qemu_wrapper);
    qemu_wrapper(sc_module_name name, int num_cpus, const char *cpu_model);
    virtual ~qemu_wrapper();

    void add_map(uint32_t base_address, uint32_t size);
    void add_map_dmi(uint32_t base_address, uint32_t size, void *data);

    void start_gdb_server(std::string port)
    {
        m_lib.start_gdb_server(port);
    }

    t_cpu * get_cpu(int i)
    {
        assert(i < m_cpus.size());
        return m_cpus[i];
    }

    template<class t_dev> t_dev* create_qdev(sc_module_name n)
    {
        t_dev *ret = new t_dev(n, &m_lib);
        ret->set_int_ctrl(m_cpus[0]);
        m_devices.push_back(ret);
        return ret;
    }
};

template <class t_cpu>
qemu_wrapper<t_cpu>::qemu_wrapper(sc_module_name name,
        int num_cpu, const char *cpu_model)
        : Master(name)
        , m_lib(t_cpu::get_iss_lib())
{
    m_lib.register_io_callback(this);
    m_lib.init(num_cpu, cpu_model);

    for (int i = 0; i < num_cpu; i++) {
        std::stringstream ss;
        ss << "cpu" << i;
        m_cpus.push_back(new t_cpu(ss.str().c_str(), i, &m_lib));
    }

    SC_THREAD(cpus_thread);
}

template<class t_cpu>
qemu_wrapper<t_cpu>::~qemu_wrapper()
{
    typename std::vector<t_cpu*>::iterator it;

    for (it = m_cpus.begin(); it !=m_cpus.end(); it++)
        delete *it;
}

template<class t_cpu>
void qemu_wrapper<t_cpu>::add_map(uint32_t base_address, uint32_t size)
{
    m_lib.map_io(base_address, size);
}

template<class t_cpu>
void 
qemu_wrapper<t_cpu>::add_map_dmi(uint32_t base_address, uint32_t size, void *data)
{
    m_lib.map_dmi(base_address, size, data);
}

#undef MODNAME

#endif
