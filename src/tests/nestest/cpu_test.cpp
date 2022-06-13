#include "gtest/gtest.h"

#include <string>
#include <fstream>
#include <unordered_set>
#include <memory>

#include "common/path.hpp"
#include "console/emulator.hpp"
#include "common/logger.hpp"

static std::ifstream
pvt_open_log(const std::string &i_path, bool &o_ok);
static bool
pvt_compare_log_line(std::ifstream *io_file, const ln::CPU *i_cpu,
                     std::size_t i_instr, ln::Cycle i_base_cycle);
static void
pvt_close_log(std::ifstream &o_file);

static bool
pvt_same_cpu_state(const ln::CPU *lhs, const ln::CPU *rhs);

TEST(cpu_test, cpu_test)
{
    ln::init_logger(spdlog::level::trace);

    auto step_emulator = std::unique_ptr<ln::Emulator>(new ln::Emulator());
    auto cycle_emulator = std::unique_ptr<ln::Emulator>(new ln::Emulator());

    auto rom_path = ln::join_exec_rel_path("nestest.nes");
    ASSERT_FALSE(LN_FAILED(step_emulator->insert_cartridge(rom_path)));
    ASSERT_FALSE(LN_FAILED(cycle_emulator->insert_cartridge(rom_path)));

    step_emulator->power_up();
    cycle_emulator->power_up();

    struct TestContext {
        std::ifstream *log_file = nullptr;
        ln::Emulator *cycle_emu = nullptr;

        bool ok = false; // test ok
        std::unordered_set<ln::Byte> opcode_set;

        ln::Cycle prev_cycle = 0;
    } context;

    auto exit_func = [](const ln::CPU *i_cpu, std::size_t i_instr,
                        void *i_context) -> bool {
        auto context = (TestContext *)i_context;

        // successfully executing all instruction without errors
        // except that the last instruction is not executed but printed only.
        if (i_instr >= 8991)
        {
            context->ok = true;
            return true;
        }

        constexpr ln::Cycle CYC_BASE = 7;
        // log the process
        auto bytes = i_cpu->get_instr_bytes(i_cpu->get_pc());
        LN_LOG_TRACE(
            ln::get_logger(), "Instruction {:>4}: ${:04X} {:02X} {} {}",
            i_instr + 1, i_cpu->get_pc(), bytes[0],
            bytes.size() >= 2 ? fmt::format("{:02X}", bytes[1]) : "  ",
            bytes.size() >= 3 ? fmt::format("{:02X}", bytes[2]) : "  ");
        LN_LOG_TRACE(ln::get_logger(),
                     "A:{:02X} X:{:02X} Y:{:02X} "
                     "P:{:02X} SP:{:02X}",
                     i_cpu->get_a(), i_cpu->get_x(), i_cpu->get_y(),
                     i_cpu->get_p(), i_cpu->get_s());
        LN_LOG_TRACE(ln::get_logger(), "CYC: {}",
                     CYC_BASE + i_cpu->get_cycle());
        // log opcode coverage
        context->opcode_set.insert(bytes[0]);

        if (!pvt_compare_log_line(context->log_file, i_cpu, i_instr, CYC_BASE))
        {
            return true;
        }

        /* Test cycle emulator by the way */
        if (i_instr >= 1)
        {
            // advance the cycles we spent
            auto cycle_diff = i_cpu->get_cycle() - context->prev_cycle;
            if (!cycle_diff)
            {
                return true;
            }
            for (decltype(cycle_diff) i = 0; i < cycle_diff; ++i)
            {
                auto cycles = context->cycle_emu->tick_cpu_test();
                if (i + 1 != cycle_diff)
                {
                    if (cycles)
                    {
                        return true;
                    }
                }
                else
                {
                    if (cycles != cycle_diff)
                    {
                        return true;
                    }
                }
            }
            // check equality
            if (!pvt_same_cpu_state(i_cpu, &context->cycle_emu->get_cpu()))
            {
                return true;
            }
        }
        context->prev_cycle = i_cpu->get_cycle();

        return false;
    };

    bool open = false;
    auto log = pvt_open_log(ln::join_exec_rel_path("nestest.log"), open);
    ASSERT_TRUE(open);
    context.log_file = &log;

    constexpr ln::Address ENTRY = 0xC000;
    cycle_emulator->run_test(ENTRY, 0, 0); // set entry function only.
    context.cycle_emu = cycle_emulator.get();

    step_emulator->run_test(ENTRY, exit_func, &context);
    ASSERT_TRUE(context.ok);

    LN_LOG_INFO(ln::get_logger(), "Opcode kinds covered: {}",
                context.opcode_set.size());

    pvt_close_log(log);
}

std::ifstream
pvt_open_log(const std::string &i_path, bool &o_ok)
{
    std::ifstream istrm(i_path, std::ios::binary);
    o_ok = istrm.is_open();
    return istrm;
}

bool
pvt_compare_log_line(std::ifstream *io_file, const ln::CPU *i_cpu,
                     std::size_t i_instr, ln::Cycle i_base_cycle)
{
    std::string line;
    std::getline(*io_file, line);
    if (line.empty())
    {
        LN_LOG_ERROR(ln::get_logger(), "Empty log line.");
        return false;
    }

    // instruction bytes
    {
        auto bytes = i_cpu->get_instr_bytes(i_cpu->get_pc());
        std::string actual = fmt::format(
            "{:04X}  {:02X} {} {}", i_cpu->get_pc(), bytes[0],
            bytes.size() >= 2 ? fmt::format("{:02X}", bytes[1]) : "  ",
            bytes.size() >= 3 ? fmt::format("{:02X}", bytes[2]) : "  ");
        // [0, 14)
        std::string expected = line.substr(0, 14);

        if (actual != expected)
        {
            LN_LOG_ERROR(ln::get_logger(),
                         "Diff failed at Instruction {}: [{}]", i_instr + 1,
                         actual);
            return false;
        }
    }
    // register state
    {
        std::string actual =
            fmt::format("A:{:02X} X:{:02X} Y:{:02X} "
                        "P:{:02X} SP:{:02X}",
                        i_cpu->get_a(), i_cpu->get_x(), i_cpu->get_y(),
                        i_cpu->get_p(), i_cpu->get_s());
        // [48, 73)
        std::string expected = line.substr(48, 73 - 48);

        if (actual != expected)
        {
            LN_LOG_ERROR(ln::get_logger(),
                         "Diff failed at Instruction {}: [{}]", i_instr + 1,
                         actual);
            return false;
        }
    }
    // cycle
    {
        std::string actual = std::to_string(i_base_cycle + i_cpu->get_cycle());
        std::string expected = line.substr(line.rfind("CYC:") + 4);
        expected.pop_back(); // '\r'
        if (actual != expected)
        {
            LN_LOG_ERROR(ln::get_logger(),
                         "Diff failed at Instruction {}: [{}] cycles",
                         i_instr + 1, expected);
            return false;
        }
    }

    return true;
}

void
pvt_close_log(std::ifstream &o_file)
{
    o_file.close();
}

bool
pvt_same_cpu_state(const ln::CPU *lhs, const ln::CPU *rhs)
{
    if (!(lhs->get_a() == rhs->get_a() && lhs->get_x() == rhs->get_x() &&
          lhs->get_y() == rhs->get_y() && lhs->get_p() == rhs->get_p() &&
          lhs->get_s() == rhs->get_s()))
    {
        return false;
    }
    if (lhs->get_pc() != rhs->get_pc())
    {
        return false;
    }
    else if (lhs->get_instr_bytes(lhs->get_pc()) !=
             rhs->get_instr_bytes(rhs->get_pc()))
    {
        return false;
    }

    return true;
}
