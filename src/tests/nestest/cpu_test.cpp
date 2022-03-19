#include "gtest/gtest.h"

#include <string>
#include <fstream>
#include <unordered_set>

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

TEST(cpu_test, cpu_test)
{
    ln::init_logger(spdlog::level::trace);

    ln::Emulator emulator;

    auto rom_path = ln::join_exec_rel_path("nestest.nes");
    ASSERT_FALSE(LN_FAILED(emulator.insert_cartridge(rom_path)));

    emulator.power_up();

    struct TestContext {
        bool ok = false; // test ok
        std::ifstream *log_file = nullptr;
        std::unordered_set<ln::Byte> opcode_set;
    } context;

    auto init_func = [](ln::Memory *i_memory, void *i_context) -> void {
        (void)(i_context);
        i_memory->set_byte(0x0002, 0xFF);
        i_memory->set_byte(0x0003, 0xFF);
    };
    auto exit_func = [](const ln::CPU *i_cpu, const ln::Memory *i_memory,
                        std::size_t i_instr, void *i_context) -> bool {
        (void)(i_memory);

        auto context = (TestContext *)i_context;

        // successfully executing all instruction without errors
        if (i_instr >= 8991)
        {
            context->ok = true;
            return true;
        }

        constexpr ln::Cycle CYC_BASE = 7;
        // log the process
        auto bytes = i_cpu->get_instruction_bytes(i_cpu->get_pc());
        ln::get_logger()->trace(
            "Instruction {:>4}: ${:04X} {:02X} {} {}", i_instr + 1,
            i_cpu->get_pc(), bytes[0],
            bytes.size() >= 2 ? fmt::format("{:02X}", bytes[1]) : "  ",
            bytes.size() >= 3 ? fmt::format("{:02X}", bytes[2]) : "  ");
        ln::get_logger()->trace("A:{:02X} X:{:02X} Y:{:02X} "
                                "P:{:02X} SP:{:02X}",
                                i_cpu->get_a(), i_cpu->get_x(), i_cpu->get_y(),
                                i_cpu->get_p(), i_cpu->get_s());
        ln::get_logger()->trace("CYC: {}", CYC_BASE + i_cpu->get_cycle());
        // log opcode coverage
        context->opcode_set.insert(bytes[0]);

        if (!pvt_compare_log_line(context->log_file, i_cpu, i_instr, CYC_BASE))
        {
            return true;
        }

        return false;
    };

    bool open = false;
    auto log = pvt_open_log(ln::join_exec_rel_path("nestest.log"), open);
    ASSERT_TRUE(open);
    context.log_file = &log;

    emulator.run_test(0xC000, init_func, exit_func, &context);
    ASSERT_TRUE(context.ok);

    ln::get_logger()->info("Opcode count covered: {}",
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
        ln::get_logger()->error("Empty log line.");
        return false;
    }

    // instruction bytes
    {
        auto bytes = i_cpu->get_instruction_bytes(i_cpu->get_pc());
        std::string actual = fmt::format(
            "{:04X}  {:02X} {} {}", i_cpu->get_pc(), bytes[0],
            bytes.size() >= 2 ? fmt::format("{:02X}", bytes[1]) : "  ",
            bytes.size() >= 3 ? fmt::format("{:02X}", bytes[2]) : "  ");
        // [0, 14)
        std::string expected = line.substr(0, 14);

        if (actual != expected)
        {
            ln::get_logger()->error("Diff failed at Instruction {}: [{}]",
                                    i_instr + 1, actual);
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
            ln::get_logger()->error("Diff failed at Instruction {}: [{}]",
                                    i_instr + 1, actual);
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
            ln::get_logger()->error(
                "Diff failed at Instruction {}: [{}] cycles", i_instr + 1,
                expected);
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
