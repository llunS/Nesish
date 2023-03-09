#include "gtest/gtest.h"

#include <string>
#include <fstream>
#include <unordered_set>
#include <memory>

#include "common/path.hpp"
#include "console/emulator.hpp"
#include "common/logger.hpp"

static std::ifstream
pv_open_log(const std::string &i_path, bool &o_ok);
static bool
pv_compare_log_line(std::ifstream *io_file, const nh::CPU *i_cpu,
                    std::size_t i_instr, nh::Cycle i_base_cycle);
static void
pv_close_log(std::ifstream &o_file);

TEST(cpu, nestest)
{
    nh::init_logger(spdlog::level::trace);

    auto cycle_emu = std::unique_ptr<nh::Emulator>(new nh::Emulator());
    auto rom_path = nh::join_exec_rel_path("nestest.nes");
    ASSERT_FALSE(LN_FAILED(cycle_emu->insert_cartridge(rom_path)));
    cycle_emu->power_up();

    bool open = false;
    auto log_file = pv_open_log(nh::join_exec_rel_path("nestest.log"), open);
    ASSERT_TRUE(open);

    auto init_func = [](nh::CPU *io_cpu, void *) {
        constexpr nh::Address ENTRY = 0xC000;
        io_cpu->set_entry_test(ENTRY);

        io_cpu->set_p_test(0x24);
    };
    cycle_emu->init_test(init_func, 0);

    bool init_log = true;
    std::unordered_set<nh::Byte> opcode_set;
    std::size_t i_instr = 0;
    while (i_instr < 8990)
    {
        auto cmp_log = [&]() {
            const auto &i_cpu = cycle_emu->get_cpu_test();

            constexpr nh::Cycle CYC_BASE = 7;
            // log the process
            auto bytes = i_cpu.get_instr_bytes(i_cpu.get_pc());
            LN_LOG_TRACE(
                nh::get_logger(), "Instruction {:>4}: ${:04X} {:02X} {} {}",
                i_instr + 1, i_cpu.get_pc(), bytes[0],
                bytes.size() >= 2 ? fmt::format("{:02X}", bytes[1]) : "  ",
                bytes.size() >= 3 ? fmt::format("{:02X}", bytes[2]) : "  ");
            LN_LOG_TRACE(nh::get_logger(),
                         "A:{:02X} X:{:02X} Y:{:02X} "
                         "P:{:02X} SP:{:02X}",
                         i_cpu.get_a(), i_cpu.get_x(), i_cpu.get_y(),
                         i_cpu.get_p(), i_cpu.get_s());
            LN_LOG_TRACE(nh::get_logger(), "CYC: {}",
                         CYC_BASE + i_cpu.get_cycle());
            // log opcode coverage
            opcode_set.insert(bytes[0]);

            if (!pv_compare_log_line(&log_file, &i_cpu, i_instr, CYC_BASE))
            {
                ASSERT_TRUE(false);
            }
        };

        if (init_log)
        {
            cmp_log();
            init_log = false;
        }
        bool cpu_instr_done = false;
        cycle_emu->tick(&cpu_instr_done);
        if (cpu_instr_done)
        {
            ++i_instr;
            cmp_log();
        }
    }

    LN_LOG_INFO(nh::get_logger(), "Opcode kinds covered: {}",
                opcode_set.size());
    pv_close_log(log_file);
}

std::ifstream
pv_open_log(const std::string &i_path, bool &o_ok)
{
    std::ifstream istrm(i_path, std::ios::binary);
    o_ok = istrm.is_open();
    return istrm;
}

bool
pv_compare_log_line(std::ifstream *io_file, const nh::CPU *i_cpu,
                    std::size_t i_instr, nh::Cycle i_base_cycle)
{
    std::string line;
    std::getline(*io_file, line);
    if (line.empty())
    {
        LN_LOG_ERROR(nh::get_logger(), "Empty log line.");
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
            LN_LOG_ERROR(nh::get_logger(),
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
            LN_LOG_ERROR(nh::get_logger(),
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
            LN_LOG_ERROR(nh::get_logger(),
                         "Diff failed at Instruction {}: [{}] cycles",
                         i_instr + 1, expected);
            return false;
        }
    }

    return true;
}

void
pv_close_log(std::ifstream &o_file)
{
    o_file.close();
}
