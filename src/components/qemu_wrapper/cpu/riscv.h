#ifndef _QEMU_WRAPPER_CPU_RISCV_H
#define _QEMU_WRAPPER_CPU_RISCV_H

#include <string>

#include "qemu_cpu.h"

class qemu_cpu_riscv : public qemu_cpu {
public:
    qemu_cpu_riscv(sc_module_name n, int cpuid, qemu_lib_wrapper *lib);

    static std::string get_iss_lib() { return "libqemu-system-riscv.so"; }

    virtual bool check_irq_idx(int idx) { return idx < 8; }
};

#endif
