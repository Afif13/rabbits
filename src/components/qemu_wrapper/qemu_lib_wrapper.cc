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

#include "qemu_lib_wrapper.h"

#define MODNAME "qemu-lib-wrapper"
#include "utils/utils.h"

#include <dlfcn.h>
#include <cassert>
#include <sc_qemu.h>

qemu_lib_wrapper::qemu_lib_wrapper(std::string lib_path)
{
    m_qemu_ctx = NULL;
    m_qemu_import = NULL;

    m_qemu_annotation = NULL;
    m_qemu_icache = NULL;
    m_qemu_dcache = NULL;

    m_lib_hdl = dlopen(lib_path.c_str(), RTLD_NOW);
    if (!m_lib_hdl) {
        EPRINTF("Cannot load library %s: %s\n",
                lib_path.c_str(), dlerror());
        exit(1);
    }
}
qemu_lib_wrapper::~qemu_lib_wrapper()
{
/*    if(m_qemu_annotation)
        delete m_qemu_annotation;
*/
}
/* ----------------------------
 * Callbacks from sc-qemu libs
 * ---------------------------- */
uint32_t qemu_lib_wrapper::qemu_sc_read(void *opaque, uint32_t addr,
                                        uint32_t size)
{
    qemu_lib_wrapper *w = (qemu_lib_wrapper *) opaque;

    return w->m_io_cb->qemu_io_read(addr, size);
}

void qemu_lib_wrapper::qemu_sc_write(void *opaque, uint32_t addr,
                                     uint32_t val, uint32_t size)
{
    qemu_lib_wrapper *w = (qemu_lib_wrapper *) opaque;

    w->m_io_cb->qemu_io_write(addr, val, size);
}

int64_t qemu_lib_wrapper::qemu_sc_call_rabbits(void *opaque, int type,
                                            int cpu, unsigned long p1,
                                            int64_t val, uint8_t size)
{
    qemu_lib_wrapper *w = (qemu_lib_wrapper *) opaque;
    switch(type) {
        case 0: //Info
            w->m_qemu_icache->info();
            w->m_qemu_dcache->info();
            w->m_qemu_annotation->info();
            break;
        case 1: //Annotation
            w->m_qemu_annotation->update_cpu_cycles(cpu, p1);
            break;
        case 2: //Sync
            w->m_qemu_icache->consume_cpu_cycles();
            w->m_qemu_dcache->consume_cpu_cycles();
            w->m_qemu_annotation->consume_cpu_cycles();
            break;
        case 3: //I Cache
            w->m_qemu_icache->icache_access(cpu, p1, qemu_sc_read, w);
            break;
        case 4: //D Cache Read
            switch(size) {
                case 1:
                    return w->m_qemu_dcache->dcache_read_b(cpu, p1,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
                case 2:
                    return w->m_qemu_dcache->dcache_read_w(cpu, p1,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
                case 4:
                    return w->m_qemu_dcache->dcache_read_l(cpu, p1,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
                case 8:
                    return w->m_qemu_dcache->dcache_read_q(cpu, p1,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
            }
            break;
        case 5: //D Cache Write
            switch(size) {
                case 1:
                    w->m_qemu_dcache->dcache_write_b(cpu, p1, val,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
                case 2:
                    w->m_qemu_dcache->dcache_write_w(cpu, p1, val,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
                case 4:
                    w->m_qemu_dcache->dcache_write_l(cpu, p1, val,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
                case 8:
                    w->m_qemu_dcache->dcache_write_q(cpu, p1, val,
                                            qemu_sc_read, qemu_sc_write,
                                            w, 1);
                    break;
            }
            break;
    }

}
/* ---------------------------- */

void qemu_lib_wrapper::init(int num_cpu, std::string cpu_model)
{
    sc_qemu_init_struct s;
    sc_qemu_init_fn qemu_init = NULL;

    m_qemu_import = new qemu_import;

    qemu_init = (sc_qemu_init_fn) dlsym(m_lib_hdl, SC_QEMU_INIT_SYM_STR);
    if (!qemu_init) {
        EPRINTF("Cannot load %s symbol from library\n",
                SC_QEMU_INIT_SYM_STR);
        exit(1);
    }

    assert(m_io_cb != NULL);

    s.sc_import.write = qemu_sc_write;
    s.sc_import.read = qemu_sc_read;
    s.sc_import.call_rabbits = qemu_sc_call_rabbits;

    s.q_import = m_qemu_import;
    s.cpu_model = cpu_model.c_str();
    s.num_cpu = num_cpu;
    s.opaque = this;

    m_qemu_ctx = qemu_init(&s);

    m_qemu_annotation = new qemu_annotation("BETA_ANNOTATION",num_cpu);
    m_qemu_icache = new qemu_icache("BETA_ICACHE",num_cpu);
    m_qemu_dcache = new qemu_dcache("BETA_DCACHE",num_cpu);
}

void qemu_lib_wrapper::map_io(uint32_t base, uint32_t size)
{
    m_qemu_import->map_io(m_qemu_ctx, base, size);
}

void qemu_lib_wrapper::map_dmi(uint32_t base, uint32_t size, void *data)
{
    m_qemu_import->map_dmi(m_qemu_ctx, base, size, data);
}

bool qemu_lib_wrapper::cpus_loop(void)
{
    return m_qemu_import->cpu_loop(m_qemu_ctx);
}

sc_qemu_qdev * qemu_lib_wrapper::cpu_get_qdev(int cpu_idx)
{
    return m_qemu_import->cpu_get_qdev(m_qemu_ctx, cpu_idx);
}

void qemu_lib_wrapper::start_gdb_server(std::string port)
{
    m_qemu_import->start_gdbserver(m_qemu_ctx, port.c_str());
}

/* 
 * QEMU devices 
 */
sc_qemu_qdev * qemu_lib_wrapper::qdev_create_cortex_a15_priv(int num_int)
{
    uint32_t i = num_int;
    return m_qemu_import->qdev_create(m_qemu_ctx, SC_QDEV_ARM_A15PRIV, i);
}

void qemu_lib_wrapper::qdev_create_uart_16550(uint32_t base_addr, int reg_shift,
                                              sc_qemu_qdev *irq_dev, int irq_idx,
                                              int baudbase)
{
    m_qemu_import->qdev_create(m_qemu_ctx, SC_QDEV_16550, base_addr, reg_shift,
                               irq_dev, irq_idx, baudbase);
}

sc_qemu_qdev * qemu_lib_wrapper::qdev_create_sp804()
{
    return m_qemu_import->qdev_create(m_qemu_ctx, SC_QDEV_SP804);
}

void qemu_lib_wrapper::qdev_destroy(sc_qemu_qdev *dev)
{}

void qemu_lib_wrapper::qdev_mmio_map(sc_qemu_qdev *dev, int mmio_id, uint32_t addr)
{
    m_qemu_import->qdev_mmio_map(dev, mmio_id, addr);
}

void qemu_lib_wrapper::qdev_irq_connect(sc_qemu_qdev *src, int src_idx,
                                        sc_qemu_qdev *dst, int dst_idx)
{
    m_qemu_import->qdev_irq_connect(src, src_idx, dst, dst_idx);
}

void qemu_lib_wrapper::qdev_irq_update(sc_qemu_qdev *dev, int irq_idx, int level)
{
    m_qemu_import->qdev_irq_update(dev, irq_idx, level);
}

void qemu_lib_wrapper::register_io_callback(qemu_io_callbacks *cb)
{
    m_io_cb = cb;
}

sc_qemu_char_dev * qemu_lib_wrapper::char_dev_create()
{
    sc_qemu_char_dev * ret;
    ret = m_qemu_import->char_dev_create(m_qemu_ctx);

    return ret;
}

int qemu_lib_wrapper::char_dev_write(sc_qemu_char_dev *dev, const uint8_t *data, int len)
{
    return m_qemu_import->char_dev_write(dev, data, len);
}

void qemu_lib_wrapper::char_dev_read(void *opaque, const uint8_t *data, int len)
{
    qemu_char_dev_callbacks *cb = (qemu_char_dev_callbacks *) opaque;

    cb->qemu_char_dev_read(data, len);
}

void qemu_lib_wrapper::char_dev_register_callbacks(sc_qemu_char_dev *dev, qemu_char_dev_callbacks *cb)
{
    m_qemu_import->char_dev_register_read(dev, char_dev_read, cb);
}
