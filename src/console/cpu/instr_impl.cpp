#include "instr_impl.hpp"

#include "console/byte_utils.hpp"

#include <type_traits>

namespace nh {

void
CPU::InstrImpl::frm_brk(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            throw_away(io_cpu, io_cpu->get_byte(io_cpu->PC));
            if (!io_cpu->m_irq_pc_no_inc)
            {
                ++io_cpu->PC;
            }
        }
        break;

        case 1:
        {
            io_cpu->push_byte(get_high(io_cpu->PC));
        }
        break;

        case 2:
        {
            io_cpu->push_byte(get_low(io_cpu->PC));
        }
        break;

        case 3:
        {
            // https://wiki.nesdev.org/w/index.php?title=Status_flags#The_B_flag
            // Push B flag differently
            Byte pushed = io_cpu->P;
            if (io_cpu->in_hardware_irq())
            {
                pushed &= Byte(~StatusFlag::B);
            }
            else
            {
                pushed |= Byte(StatusFlag::B);
            }
            io_cpu->push_byte(pushed);

            // Interrupt hijacking
            // Determine vector address at this cycle
            // Priority: RESET(?)>NMI>IRQ>BRK
            // Check using the current signal status, instead of what
            // initiates the interrupt sequence.
            // The hijacking doesn't lose the B flag
            // RESET
            if (io_cpu->m_reset_sig)
            {
                io_cpu->m_addr_bus = Memory::RESET_VECTOR_ADDR;
            }
            // NMI
            else if (io_cpu->m_nmi_sig)
            {
                io_cpu->m_addr_bus = Memory::NMI_VECTOR_ADDR;
            }
            // Since IRQ and BRK use the same vector, don't bother to check
            else
            {
                io_cpu->m_addr_bus = Memory::IRQ_VECTOR_ADDR;
            }
        }
        break;

        case 4:
        {
            set_low(io_cpu->PC, io_cpu->get_byte(io_cpu->m_addr_bus));
            // Set I flag at this cycle according to doc:
            // https://www.nesdev.org/wiki/CPU_interrupts#Interrupt_hijacking
            io_cpu->set_flag(CPU::StatusFlag::I);
        }
        break;

        case 5:
        {
            ++io_cpu->m_addr_bus;
            set_high(io_cpu->PC, io_cpu->get_byte(io_cpu->m_addr_bus));

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_rti(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            throw_away(io_cpu, io_cpu->get_byte(io_cpu->PC));
        }
        break;

        case 1:
        {
            io_cpu->pre_pop_byte();
        }
        break;

        case 2:
        {
            // Disregards bits 5 and 4 when reading flags from the stack
            // https://www.nesdev.org/wiki/Status_flags#The_B_flag
            ignore_ub(io_cpu->post_pop_byte(), io_cpu->P);
            io_cpu->pre_pop_byte();
        }
        break;

        case 3:
        {
            set_low(io_cpu->PC, io_cpu->post_pop_byte());
            io_cpu->pre_pop_byte();
        }
        break;

        case 4:
        {
            set_high(io_cpu->PC, io_cpu->post_pop_byte());

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_rts(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            throw_away(io_cpu, io_cpu->get_byte(io_cpu->PC));
        }
        break;

        case 1:
        {
            io_cpu->pre_pop_byte();
        }
        break;

        case 2:
        {
            set_low(io_cpu->PC, io_cpu->post_pop_byte());
            io_cpu->pre_pop_byte();
        }
        break;

        case 3:
        {
            set_high(io_cpu->PC, io_cpu->post_pop_byte());
        }
        break;

        case 4:
        {
            ++io_cpu->PC;

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_phr_impl(int i_idx, CPU *io_cpu, InstrCore i_core,
                             bool &io_done, Byte i_val)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            throw_away(io_cpu, io_cpu->get_byte(io_cpu->PC));
        }
        break;

        case 1:
        {
            io_cpu->push_byte(i_val);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_pha(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    frm_phr_impl(i_idx, io_cpu, i_core, io_done, io_cpu->A);
}

void
CPU::InstrImpl::frm_php(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    // https://wiki.nesdev.org/w/index.php?title=Status_flags#The_B_flag
    // Push the status register with the B flag set
    frm_phr_impl(i_idx, io_cpu, i_core, io_done, io_cpu->P | StatusFlag::B);
}

void
CPU::InstrImpl::frm_plr_impl(int i_idx, CPU *io_cpu, InstrCore i_core,
                             bool &io_done, Byte &o_val)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            throw_away(io_cpu, io_cpu->get_byte(io_cpu->PC));
        }
        break;

        case 1:
        {
            io_cpu->pre_pop_byte();
        }
        break;

        case 2:
        {
            o_val = io_cpu->post_pop_byte();

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_pla(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    Byte val;
    frm_plr_impl(i_idx, io_cpu, i_core, io_done, val);
    if (io_done)
    {
        io_cpu->A = val;

        test_flag_n(io_cpu, io_cpu->A);
        test_flag_z(io_cpu, io_cpu->A);
    }
}

void
CPU::InstrImpl::frm_plp(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    Byte val;
    frm_plr_impl(i_idx, io_cpu, i_core, io_done, val);
    if (io_done)
    {
        // Disregards bits 5 and 4 when reading flags from the stack
        // https://www.nesdev.org/wiki/Status_flags#The_B_flag
        ignore_ub(val, io_cpu->P);
    }
}

void
CPU::InstrImpl::frm_jsr(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->PC++);
        }
        break;

        case 1:
        {
            // Unclear as to what internal operation is done at this cycle.
        }
        break;

        case 2:
        {
            io_cpu->push_byte(get_high(io_cpu->PC));
        }
        break;

        case 3:
        {
            io_cpu->push_byte(get_low(io_cpu->PC));
        }
        break;

        case 4:
        {
            // Fetch high address byte before mutating PC
            Byte high = io_cpu->get_byte(io_cpu->PC);
            set_low(io_cpu->PC, io_cpu->m_data_bus);
            set_high(io_cpu->PC, high);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_imp(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    // @TODO: Pipelining

    // Since the in and out won't be used in core for this kind of
    // instructions, reuse acc implemenation for simplicity.
    frm_acc(i_idx, io_cpu, i_core, io_done);
}

void
CPU::InstrImpl::frm_acc(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            throw_away(io_cpu, io_cpu->get_byte(io_cpu->PC));

            // @TODO: Pipelining
            i_core(io_cpu, io_cpu->A, io_cpu->A);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_imm(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            // @TODO: Pipelining
            Byte out;
            i_core(io_cpu, io_cpu->get_byte(io_cpu->PC++), out);
            (void)(out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abs_jmp(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->PC++);
        }
        break;

        case 1:
        {
            // Fetch high address byte before mutating PC
            Byte high = io_cpu->get_byte(io_cpu->PC);
            set_low(io_cpu->PC, io_cpu->m_data_bus);
            set_high(io_cpu->PC, high);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abs_pre(int i_idx, CPU *io_cpu)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            set_low(io_cpu->m_addr_bus, io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            set_high(io_cpu->m_addr_bus, io_cpu->get_byte(io_cpu->PC++));
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abs_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_abs_pre(i_idx, io_cpu);
        }
        break;

        case 2:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->get_byte(io_cpu->m_addr_bus), out);
            (void)(out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abs_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_abs_pre(i_idx, io_cpu);
        }
        break;

        case 2:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 3:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->m_data_bus, out);
            io_cpu->m_data_bus = out;
        }
        break;

        case 4:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abs_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_abs_pre(i_idx, io_cpu);
        }
        break;

        case 2:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, 0, out);
            io_cpu->set_byte(io_cpu->m_addr_bus, out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abi_pre(int i_idx, CPU *io_cpu, Byte i_val)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            set_low(io_cpu->m_addr_bus, io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            set_high(io_cpu->m_addr_bus, io_cpu->get_byte(io_cpu->PC++));
            Address new_addr = get_low(io_cpu->m_addr_bus) + i_val;
            io_cpu->m_page_offset = bool(new_addr & 0xFF00);
            set_low(io_cpu->m_addr_bus, get_low(new_addr));
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abi_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done, Byte i_val)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_abi_pre(i_idx, io_cpu, i_val);
        }
        break;

        case 2:
        {
            // The read must be done regardlessly.
            Byte in = io_cpu->get_byte(io_cpu->m_addr_bus);
            if (io_cpu->m_page_offset)
            {
                io_cpu->m_addr_bus += (Address(io_cpu->m_page_offset) << 8);
            }
            else
            {
                io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
                Byte out;
                i_core(io_cpu, in, out);
                (void)(out);

                io_done = true;
            }
        }
        break;

        case 3:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->get_byte(io_cpu->m_addr_bus), out);
            (void)(out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abi_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done, Byte i_val)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_abi_pre(i_idx, io_cpu, i_val);
        }
        break;

        case 2:
        {
            // Read from possibly smaller address first.
            (void)(io_cpu->get_byte(io_cpu->m_addr_bus));
            // if (io_cpu->m_page_offset)
            {
                io_cpu->m_addr_bus += (Address(io_cpu->m_page_offset) << 8);
            }
        }
        break;

        case 3:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, 0, out);
            io_cpu->set_byte(io_cpu->m_addr_bus, out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abi_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done, Byte i_val)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_abi_pre(i_idx, io_cpu, i_val);
        }
        break;

        case 2:
        {
            // Read from possibly smaller address first.
            (void)(io_cpu->get_byte(io_cpu->m_addr_bus));
            // if (io_cpu->m_page_offset)
            {
                io_cpu->m_addr_bus += (Address(io_cpu->m_page_offset) << 8);
            }
        }
        break;

        case 3:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 4:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->m_data_bus, out);
            io_cpu->m_data_bus = out;
        }
        break;

        case 5:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_abx_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_abi_r(i_idx, io_cpu, i_core, io_done, io_cpu->X);
}

void
CPU::InstrImpl::frm_abx_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    frm_abi_rmw(i_idx, io_cpu, i_core, io_done, io_cpu->X);
}

void
CPU::InstrImpl::frm_abx_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_abi_w(i_idx, io_cpu, i_core, io_done, io_cpu->X);
}

void
CPU::InstrImpl::frm_aby_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_abi_r(i_idx, io_cpu, i_core, io_done, io_cpu->Y);
}

void
CPU::InstrImpl::frm_aby_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    frm_abi_rmw(i_idx, io_cpu, i_core, io_done, io_cpu->Y);
}

void
CPU::InstrImpl::frm_aby_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_abi_w(i_idx, io_cpu, i_core, io_done, io_cpu->Y);
}

void
CPU::InstrImpl::frm_zp_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                         bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_addr_bus = Address(io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->get_byte(io_cpu->m_addr_bus), out);
            (void)(out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_zp_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                           bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_addr_bus = Address(io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 2:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->m_data_bus, out);
            io_cpu->m_data_bus = out;
        }
        break;

        case 3:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_zp_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                         bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_addr_bus = Address(io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, 0, out);
            io_cpu->set_byte(io_cpu->m_addr_bus, out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_zpi_pre(int i_idx, CPU *io_cpu, Byte i_val)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_addr_bus = Address(io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            (void)(io_cpu->get_byte(io_cpu->m_addr_bus));
            io_cpu->m_addr_bus = (io_cpu->m_addr_bus + i_val) & 0x00FF;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_zpi_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done, Byte i_val)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_zpi_pre(i_idx, io_cpu, i_val);
        }
        break;

        case 2:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->get_byte(io_cpu->m_addr_bus), out);
            (void)(out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_zpi_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done, Byte i_val)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_zpi_pre(i_idx, io_cpu, i_val);
        }
        break;

        case 2:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, 0, out);
            io_cpu->set_byte(io_cpu->m_addr_bus, out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_zpx_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_zpi_r(i_idx, io_cpu, i_core, io_done, io_cpu->X);
}

void
CPU::InstrImpl::frm_zpx_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        {
            frm_zpi_pre(i_idx, io_cpu, io_cpu->X);
        }
        break;

        case 2:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 3:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->m_data_bus, out);
            io_cpu->m_data_bus = out;
        }
        break;

        case 4:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_zpx_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_zpi_w(i_idx, io_cpu, i_core, io_done, io_cpu->X);
}

void
CPU::InstrImpl::frm_zpy_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_zpi_r(i_idx, io_cpu, i_core, io_done, io_cpu->Y);
}

void
CPU::InstrImpl::frm_zpy_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    frm_zpi_w(i_idx, io_cpu, i_core, io_done, io_cpu->Y);
}

void
CPU::InstrImpl::frm_rel(int i_idx, CPU *io_cpu, InstrCore i_core, bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->PC++);
            Byte out;
            i_core(io_cpu, 0, out);
            bool branch_taken = out;

            if (!branch_taken)
            {
                io_done = true;
            }
            // This is polled regardless of branching or not
            io_cpu->poll_interrupt();
            // Even if interrupt is detected, it is delayed until this
            // instruction is complete, or else the taken branch could be missed
            // after the handler returns.
            // So it's fine we poll in the middle of an instruction.
        }
        break;

        case 1:
        {
            // Branch is taken at this point

            Byte opcode = io_cpu->get_byte(io_cpu->PC);
            (void)(opcode);

            // Set PCL and calculate PCH offset, using larger type.
            // @NOTE: The conversion to SignedByte is needed, so that higher
            // bits be padded with sign bit when converted to larger unsigned
            // type.
            SignedByte offset = (SignedByte)(io_cpu->m_data_bus);
            Address new_pc = io_cpu->PC + offset;
            // Before PC is mutated
            static_assert(
                std::is_same<decltype(io_cpu->m_page_offset), Byte>::value,
                "Check correctness of arithmetic operations");
            io_cpu->m_page_offset = get_high(new_pc) - get_high(io_cpu->PC);
            set_low(io_cpu->PC, get_low(new_pc));

            if (!io_cpu->m_page_offset)
            {
                io_done = true;
            }
            // A taken non-page-crossing doesn't poll so it may delay the
            // interrupt by one instruction.
        }
        break;

        case 2:
        {
            // Branch occurs to different page at this point

            // The high byte of Program Counter (PCH) is invalid at this time,
            // i.e. it may be smaller or bigger by $0100.
            Byte opcode = io_cpu->get_byte(io_cpu->PC);
            (void)(opcode);

            // Fix PCH
            // if (io_cpu->m_page_offset)
            {
                io_cpu->PC += (Address(io_cpu->m_page_offset) << 8);
            }

            io_done = true;
            io_cpu->poll_interrupt();
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izx_pre(int i_idx, CPU *io_cpu)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_addr_bus = Address(io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            (void)(io_cpu->get_byte(io_cpu->m_addr_bus));
            io_cpu->m_addr_bus = (io_cpu->m_addr_bus + io_cpu->X) & 0x00FF;
        }
        break;

        case 2:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 3:
        {
            Byte high = io_cpu->get_byte((io_cpu->m_addr_bus + 1) & 0x00FF);
            io_cpu->m_addr_bus = to_byte2(high, io_cpu->m_data_bus);
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izx_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        case 2:
        case 3:
        {
            frm_izx_pre(i_idx, io_cpu);
        }
        break;

        case 4:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->get_byte(io_cpu->m_addr_bus), out);
            (void)(out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izx_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        case 2:
        case 3:
        {
            frm_izx_pre(i_idx, io_cpu);
        }
        break;

        case 4:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 5:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->m_data_bus, out);
            io_cpu->m_data_bus = out;
        }
        break;

        case 6:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izx_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        case 2:
        case 3:
        {
            frm_izx_pre(i_idx, io_cpu);
        }
        break;

        case 4:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, 0, out);
            io_cpu->set_byte(io_cpu->m_addr_bus, out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izy_pre(int i_idx, CPU *io_cpu)
{
    switch (i_idx)
    {
        default:
        case 0:
        {
            io_cpu->m_addr_bus = Address(io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 2:
        {
            set_high(io_cpu->m_addr_bus,
                     io_cpu->get_byte((io_cpu->m_addr_bus + 1) & 0x00FF));
            Address sum_addr = Address(io_cpu->m_data_bus) + io_cpu->Y;
            io_cpu->m_page_offset = bool(sum_addr & 0xFF00);
            set_low(io_cpu->m_addr_bus, get_low(sum_addr));
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izy_r(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        case 2:
        {
            frm_izy_pre(i_idx, io_cpu);
        }
        break;

        case 3:
        {
            // The read must be done regardlessly.
            Byte in = io_cpu->get_byte(io_cpu->m_addr_bus);
            if (io_cpu->m_page_offset)
            {
                io_cpu->m_addr_bus += (Address(io_cpu->m_page_offset) << 8);
            }
            else
            {
                io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
                Byte out;
                i_core(io_cpu, in, out);
                (void)(out);

                io_done = true;
            }
        }
        break;

        case 4:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->get_byte(io_cpu->m_addr_bus), out);
            (void)(out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izy_rmw(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        case 2:
        {
            frm_izy_pre(i_idx, io_cpu);
        }
        break;

        case 3:
        {
            // Read from possibly smaller address first.
            (void)(io_cpu->get_byte(io_cpu->m_addr_bus));
            // if (io_cpu->m_page_offset)
            {
                io_cpu->m_addr_bus += (Address(io_cpu->m_page_offset) << 8);
            }
        }
        break;

        case 4:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 5:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, io_cpu->m_data_bus, out);
            io_cpu->m_data_bus = out;
        }
        break;

        case 6:
        {
            io_cpu->set_byte(io_cpu->m_addr_bus, io_cpu->m_data_bus);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_izy_w(int i_idx, CPU *io_cpu, InstrCore i_core,
                          bool &io_done)
{
    switch (i_idx)
    {
        default:
        case 0:
        case 1:
        case 2:
        {
            frm_izy_pre(i_idx, io_cpu);
        }
        break;

        case 3:
        {
            // Read from possibly smaller address first.
            (void)(io_cpu->get_byte(io_cpu->m_addr_bus));
            // if (io_cpu->m_page_offset)
            {
                io_cpu->m_addr_bus += (Address(io_cpu->m_page_offset) << 8);
            }
        }
        break;

        case 4:
        {
            io_cpu->m_i_eff_addr = io_cpu->m_addr_bus;
            Byte out;
            i_core(io_cpu, 0, out);
            io_cpu->set_byte(io_cpu->m_addr_bus, out);

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::frm_ind_jmp(int i_idx, CPU *io_cpu, InstrCore i_core,
                            bool &io_done)
{
    (void)(i_core);

    switch (i_idx)
    {
        default:
        case 0:
        {
            set_low(io_cpu->m_addr_bus, io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 1:
        {
            set_high(io_cpu->m_addr_bus, io_cpu->get_byte(io_cpu->PC++));
        }
        break;

        case 2:
        {
            io_cpu->m_data_bus = io_cpu->get_byte(io_cpu->m_addr_bus);
        }
        break;

        case 3:
        {
            // Set PCL at this cycle, according to the doc
            set_low(io_cpu->PC, io_cpu->m_data_bus);

            // Address bytes are fetched from the same page
            io_cpu->m_addr_bus = to_byte2(get_high(io_cpu->m_addr_bus),
                                          get_low(io_cpu->m_addr_bus + 1));
            set_high(io_cpu->PC, io_cpu->get_byte(io_cpu->m_addr_bus));

            io_done = true;
        }
        break;
    }
}

void
CPU::InstrImpl::core_nop(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(io_cpu);
    (void)(i_in);
    (void)(o_out);
}

void
CPU::InstrImpl::core_ora(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    io_cpu->A |= i_in;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_kil(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    io_cpu->halt();
}

void
CPU::InstrImpl::core_asl(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    Byte new_val = i_in << 1;
    o_out = new_val;

    test_flag_n(io_cpu, new_val);
    test_flag_z(io_cpu, new_val);
    io_cpu->test_flag(StatusFlag::C, check_bit<7>(i_in));
}

void
CPU::InstrImpl::core_bpl(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = !io_cpu->check_flag(StatusFlag::N);
}

void
CPU::InstrImpl::core_clc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);
    io_cpu->unset_flag(StatusFlag::C);
}

void
CPU::InstrImpl::core_and(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    core_and_op(io_cpu, i_in);

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_bit(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    test_flag_n(io_cpu, i_in);
    io_cpu->test_flag(StatusFlag::V, check_bit<6>(i_in));
    test_flag_z(io_cpu, io_cpu->A & i_in);
}

void
CPU::InstrImpl::core_rol(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    bool carry = io_cpu->check_flag(StatusFlag::C);
    o_out = (i_in << 1) | (carry << 0);

    test_flag_n(io_cpu, o_out);
    test_flag_z(io_cpu, o_out);
    io_cpu->test_flag(StatusFlag::C, check_bit<7>(i_in));
}

void
CPU::InstrImpl::core_bmi(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->check_flag(StatusFlag::N);
}

void
CPU::InstrImpl::core_sec(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);
    io_cpu->set_flag(StatusFlag::C);
}

void
CPU::InstrImpl::core_eor(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    io_cpu->A ^= i_in;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_lsr(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    o_out = i_in >> 1;

    test_flag_n(io_cpu, o_out);
    test_flag_z(io_cpu, o_out);
    io_cpu->test_flag(StatusFlag::C, check_bit<0>(i_in));
}

void
CPU::InstrImpl::core_bvc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = !io_cpu->check_flag(StatusFlag::V);
}

void
CPU::InstrImpl::core_cli(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);
    io_cpu->unset_flag(StatusFlag::I);
}

void
CPU::InstrImpl::core_adc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    bool carry = io_cpu->check_flag(StatusFlag::C);

    Byte prev_val = io_cpu->A;
    io_cpu->A += i_in + (Byte)carry;

    // either new value < previous value, or the are the same even if carry
    // exists.
    bool unsigned_overflow =
        (io_cpu->A < prev_val || (io_cpu->A == prev_val && carry));
    bool signed_overflow = is_signed_overflow_adc(prev_val, i_in, carry);

    test_flag_n(io_cpu, io_cpu->A);
    io_cpu->test_flag(StatusFlag::V, signed_overflow);
    test_flag_z(io_cpu, io_cpu->A);
    io_cpu->test_flag(StatusFlag::C, unsigned_overflow);
}

void
CPU::InstrImpl::core_ror(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    core_ror_op(io_cpu, i_in, o_out);

    test_flag_n(io_cpu, o_out);
    test_flag_z(io_cpu, o_out);
    io_cpu->test_flag(StatusFlag::C, check_bit<0>(i_in));
}

void
CPU::InstrImpl::core_bvs(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->check_flag(StatusFlag::V);
}

void
CPU::InstrImpl::core_sei(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);
    io_cpu->set_flag(StatusFlag::I);
}

void
CPU::InstrImpl::core_sta(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->A;
}

void
CPU::InstrImpl::core_sty(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->Y;
}

void
CPU::InstrImpl::core_stx(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->X;
}

void
CPU::InstrImpl::core_dey(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    --io_cpu->Y;

    test_flag_n(io_cpu, io_cpu->Y);
    test_flag_z(io_cpu, io_cpu->Y);
}

void
CPU::InstrImpl::core_txa(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    io_cpu->A = io_cpu->X;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_bcc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = !io_cpu->check_flag(StatusFlag::C);
}

void
CPU::InstrImpl::core_tya(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    io_cpu->A = io_cpu->Y;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_txs(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    io_cpu->S = io_cpu->X;
}

void
CPU::InstrImpl::core_ldy(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    io_cpu->Y = i_in;

    test_flag_n(io_cpu, io_cpu->Y);
    test_flag_z(io_cpu, io_cpu->Y);
}

void
CPU::InstrImpl::core_lda(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    io_cpu->A = i_in;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_ldx(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    io_cpu->X = i_in;

    test_flag_n(io_cpu, io_cpu->X);
    test_flag_z(io_cpu, io_cpu->X);
}

void
CPU::InstrImpl::core_tay(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    io_cpu->Y = io_cpu->A;

    test_flag_n(io_cpu, io_cpu->Y);
    test_flag_z(io_cpu, io_cpu->Y);
}

void
CPU::InstrImpl::core_tax(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    io_cpu->X = io_cpu->A;

    test_flag_n(io_cpu, io_cpu->X);
    test_flag_z(io_cpu, io_cpu->X);
}

void
CPU::InstrImpl::core_bcs(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->check_flag(StatusFlag::C);
}

void
CPU::InstrImpl::core_clv(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);
    io_cpu->unset_flag(StatusFlag::V);
}

void
CPU::InstrImpl::core_tsx(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    io_cpu->X = io_cpu->S;

    test_flag_n(io_cpu, io_cpu->X);
    test_flag_z(io_cpu, io_cpu->X);
}

void
CPU::InstrImpl::core_cpy(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);
    (void)(core_cmp_impl(io_cpu, i_in, io_cpu->Y));
}

void
CPU::InstrImpl::core_cmp(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);
    (void)(core_cmp_impl(io_cpu, i_in, io_cpu->A));
}

void
CPU::InstrImpl::core_dec(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    o_out = i_in - 1;

    test_flag_n(io_cpu, o_out);
    test_flag_z(io_cpu, o_out);
}

void
CPU::InstrImpl::core_iny(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    ++io_cpu->Y;

    test_flag_n(io_cpu, io_cpu->Y);
    test_flag_z(io_cpu, io_cpu->Y);
}

void
CPU::InstrImpl::core_dex(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    --io_cpu->X;

    test_flag_n(io_cpu, io_cpu->X);
    test_flag_z(io_cpu, io_cpu->X);
}

void
CPU::InstrImpl::core_bne(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = !io_cpu->check_flag(StatusFlag::Z);
}

void
CPU::InstrImpl::core_cld(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);
    io_cpu->unset_flag(StatusFlag::D);
}

void
CPU::InstrImpl::core_cpx(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);
    (void)(core_cmp_impl(io_cpu, i_in, io_cpu->X));
}

void
CPU::InstrImpl::core_sbc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    bool borrow = !io_cpu->check_flag(StatusFlag::C);

    Byte prev_val = io_cpu->A;
    Byte2 minuend = io_cpu->A;
    Byte2 subtrahend = i_in + (Byte2)borrow;
    io_cpu->A = (Byte)(minuend - subtrahend);

    bool no_borrow = minuend >= subtrahend;
    bool signed_overflow = is_signed_overflow_sbc(prev_val, i_in, borrow);

    test_flag_n(io_cpu, io_cpu->A);
    io_cpu->test_flag(StatusFlag::V, signed_overflow);
    test_flag_z(io_cpu, io_cpu->A);
    io_cpu->test_flag(StatusFlag::C, no_borrow);
}

void
CPU::InstrImpl::core_inc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    o_out = i_in + 1;

    test_flag_n(io_cpu, o_out);
    test_flag_z(io_cpu, o_out);
}

void
CPU::InstrImpl::core_inx(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);

    ++io_cpu->X;

    test_flag_n(io_cpu, io_cpu->X);
    test_flag_z(io_cpu, io_cpu->X);
}

void
CPU::InstrImpl::core_beq(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->check_flag(StatusFlag::Z);
}

void
CPU::InstrImpl::core_sed(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    (void)(o_out);
    io_cpu->set_flag(StatusFlag::D);
}

void
CPU::InstrImpl::core_slo(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    core_asl(io_cpu, i_in, o_out);
    Byte tmp;
    core_ora(io_cpu, o_out, tmp);
    (void)(tmp);
}

void
CPU::InstrImpl::core_anc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.com/extra_instructions.txt

    core_and_op(io_cpu, i_in);

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
    io_cpu->test_flag(StatusFlag::C, check_bit<7>(io_cpu->A));
}

void
CPU::InstrImpl::core_rla(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    core_rol(io_cpu, i_in, o_out);
    Byte tmp;
    core_and(io_cpu, o_out, tmp);
    (void)(tmp);
}

void
CPU::InstrImpl::core_alr(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    // http://www.oxyron.de/html/opcodes02.html

    // AND
    {
        Byte tmp;
        core_and(io_cpu, i_in, tmp);
        (void)(tmp);
    }
    // LSR A
    core_lsr(io_cpu, io_cpu->A, io_cpu->A);
}

void
CPU::InstrImpl::core_sre(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    core_lsr(io_cpu, i_in, o_out);
    Byte tmp;
    core_eor(io_cpu, o_out, tmp);
    (void)(tmp);
}

void
CPU::InstrImpl::core_arr(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    // Similar to AND #i then ROR A, except sets the flags differently. N and Z
    // are normal, but C is bit 6 and V is bit 6 xor bit 5. A fast way to
    // perform signed division by 4 is: CMP #$80; ARR #$FF; ROR. This can be
    // extended to larger powers of two.

    // ROR depends on C flag, but AND doesn't affect C flag, so we can
    // reuse AND logic and do it first.
    core_and_op(io_cpu, i_in);
    Byte after_and = io_cpu->A;
    // ROR A
    core_ror_op(io_cpu, after_and, io_cpu->A);

    test_flag_n(io_cpu, io_cpu->A);
    io_cpu->test_flag(StatusFlag::V,
                      check_bit<5>(io_cpu->A) ^ check_bit<6>(io_cpu->A));
    test_flag_z(io_cpu, io_cpu->A);
    io_cpu->test_flag(StatusFlag::C, check_bit<7>(after_and));
}

void
CPU::InstrImpl::core_rra(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    core_ror(io_cpu, i_in, o_out);
    Byte tmp;
    core_adc(io_cpu, o_out, tmp);
    (void)(tmp);
}

void
CPU::InstrImpl::core_xaa(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);
    io_cpu->A = io_cpu->X & i_in;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_sax(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);
    o_out = io_cpu->A & io_cpu->X;
}

void
CPU::InstrImpl::core_las(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    Byte new_val = i_in & io_cpu->S;
    io_cpu->A = io_cpu->X = io_cpu->S = new_val;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_lax(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    io_cpu->A = i_in;
    io_cpu->X = i_in;

    test_flag_n(io_cpu, io_cpu->A);
    test_flag_z(io_cpu, io_cpu->A);
}

void
CPU::InstrImpl::core_axs(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(o_out);

    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.org/wiki/Programming_with_unofficial_opcodes#Combined_operations

    Byte a_and_x = io_cpu->A & io_cpu->X;
    Byte delta = core_cmp_impl(io_cpu, i_in, a_and_x);
    io_cpu->X = delta;
}

void
CPU::InstrImpl::core_dcp(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    core_dec(io_cpu, i_in, o_out);
    Byte tmp;
    core_cmp(io_cpu, o_out, tmp);
    (void)(tmp);
}

void
CPU::InstrImpl::core_isc(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    core_inc(io_cpu, i_in, o_out);
    Byte tmp;
    core_sbc(io_cpu, o_out, tmp);
    (void)(tmp);
}

void
CPU::InstrImpl::core_tas(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);

    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.com/extra_instructions.txt
    // @TEST: Test this instruction

    io_cpu->S = io_cpu->A & io_cpu->X;
    core_XhX_impl(io_cpu, io_cpu->S, o_out);
}

void
CPU::InstrImpl::core_shy(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);

    core_XhX_impl(io_cpu, io_cpu->Y, o_out);
}

void
CPU::InstrImpl::core_shx(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);

    core_XhX_impl(io_cpu, io_cpu->X, o_out);
}

void
CPU::InstrImpl::core_ahx(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    (void)(i_in);

    core_XhX_impl(io_cpu, io_cpu->A & io_cpu->X, o_out);
}

void
CPU::InstrImpl::core_XhX_impl(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.com/extra_instructions.txt
    // https://github.com/ltriant/nes/blob/master/doc/undocumented_opcodes.txt
    // @TEST: Test this instruction

    // Assuming effective address is set already
    o_out = i_in & (get_high(io_cpu->m_i_eff_addr) + 1);
}

Byte
CPU::InstrImpl::core_cmp_impl(CPU *io_cpu, Byte i_in, nh::Byte i_reg)
{
    Byte delta = i_reg - i_in;
    bool borrow = i_reg < i_in;

    test_flag_n(io_cpu, delta);
    test_flag_z(io_cpu, delta);
    io_cpu->test_flag(StatusFlag::C, !borrow);

    return delta;
}

void
CPU::InstrImpl::core_and_op(CPU *io_cpu, Byte i_in)
{
    io_cpu->A &= i_in;
}

void
CPU::InstrImpl::core_ror_op(CPU *io_cpu, Byte i_in, Byte &o_out)
{
    bool carry = io_cpu->check_flag(StatusFlag::C);
    o_out = (i_in >> 1) | (carry << 7);
}

void
CPU::InstrImpl::throw_away(CPU *io_cpu, Byte i_data)
{
    // What does it mean to throw it away?
    // Inferring from the wording in the doc, putting it in the bus does not
    // seem right.
    // io_cpu->m_data_bus = i_data;

    (void)(io_cpu);
    (void)(i_data);
}

void
CPU::InstrImpl::ignore_ub(Byte i_src, Byte &io_dst)
{
    Byte preserve_mask = (StatusFlag::U | StatusFlag::B);
    io_dst = (i_src & ~preserve_mask) | (io_dst & preserve_mask);
}

void
CPU::InstrImpl::set_low(Address &o_dst, Byte i_val)
{
    o_dst = (o_dst & 0xFF00) | Address(i_val);
}

void
CPU::InstrImpl::set_high(Address &o_dst, Byte i_val)
{
    o_dst = (o_dst & 0x00FF) | (Address(i_val) << 8);
}

Byte
CPU::InstrImpl::get_low(Address i_val)
{
    return Byte(i_val);
}

Byte
CPU::InstrImpl::get_high(Address i_val)
{
    return Byte(i_val >> 8);
}

void
CPU::InstrImpl::test_flag_n(CPU *o_cpu, Byte i_val)
{
    o_cpu->test_flag(StatusFlag::N, check_bit<7>(i_val));
}

void
CPU::InstrImpl::test_flag_z(CPU *o_cpu, Byte i_val)
{
    o_cpu->test_flag(StatusFlag::Z, i_val == 0);
}

} // namespace nh
