#include "riscv.h"

qemu_cpu_riscv::qemu_cpu_riscv(sc_module_name n, int cpuid, qemu_lib_wrapper *lib)
    : qemu_cpu(n, cpuid, lib)
{
}
