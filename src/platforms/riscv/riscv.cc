#include "riscv.h"
#include "utils/loader.h"

#define MODENAME "riscv-platform"
#include "utils/utils.h"

RISCVPlatform::RISCVPlatform(sc_module_name n, std::string kernel_fn) 
    : generic_subsystem(n)
    , m_kernel_fn(kernel_fn)
{
    m_cpu = new qemu_wrapper<qemu_cpu_riscv>("cpu", 1, "riscv-generic");
    m_dbg = new DebugInitiator("debug-initiator");
    m_mem = new memory("ram", MEM_SIZE);
    m_uart = m_cpu->create_qdev<qemu_uart_16550>("uart");

    m_uart->set_regshift(0);
    m_uart->set_baudbase(1843200/16);
    m_uart->connect_irq(4);

    connect_target(m_mem, 0, MEM_SIZE);
    m_cpu->add_map_dmi(0x0, MEM_SIZE, m_mem->get_mem());

    connect_initiator(m_cpu);
    connect_initiator<DebugInitiator>(&m_dbg->socket);
    m_uart->set_base_addr(0x3f8);

}

void RISCVPlatform::end_of_elaboration()
{
    uint32_t mem_size = MEM_SIZE >> 20;

    if(Loader::load_elf(m_kernel_fn, m_dbg, NULL)) {
        EPRINTF("Load elf failed\n");
        exit(1);
    }

    m_dbg->debug_write(0, &mem_size, sizeof(mem_size));
}

static void usage(const char *arg0)
{
    puts("");
    printf("Usage: %s -kernel <path>\n", arg0);
    printf("Option:\n");
    printf("  -kernel <path>\tPath to the kernel image\n\n");
    printf("Press `Ctrl-a x' during simulation to quit.\n");
}

int sc_main(int argc, char **argv)
{
    /* Minimal argument handling */
    if ((argc != 3) || (std::string(argv[1]) != "-kernel")) {
        usage(argv[0]);
        return 1;
    }

    RISCVPlatform platform("riscv-platform", argv[2]);

    sc_start();

    return 0;
}
