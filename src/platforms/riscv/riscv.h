#include "rabbits-common.h"

#include "components/generic_subsystem/generic_subsystem.h"

#include "components/qemu_wrapper/cpu/riscv.h"
#include "components/qemu_wrapper/qemu_wrapper.h"
#include "components/memory/memory.h"
#include "components/qemu_wrapper/dev/uart_16550.h"
#include "components/debug/debug_initiator.h"

class RISCVPlatform : public generic_subsystem {
protected:
    static const uint64_t MEM_SIZE = 128 * 1024 * 1024;

    qemu_wrapper<qemu_cpu_riscv> *m_cpu;
    DebugInitiator *m_dbg;

    memory *m_mem;

    qemu_uart_16550 *m_uart;

    std::string m_kernel_fn;

    virtual void end_of_elaboration();
    

public:
    RISCVPlatform(sc_module_name n, std::string kernel_fn);
};
