#include "gtest/gtest.h"

#include <string>
#include <fstream>
#include <unordered_set>
#include <cstdio>

#include "nesish/nesish.h"
#include "nhbase/path.hpp"

#include "fmt/core.h"

static std::ifstream
pv_open_log(const std::string &i_path, bool &o_ok);
static bool
pv_compare_log_line(std::ifstream *io_file, NHCPU i_cpu, std::size_t i_instr,
                    NHCycle i_base_cycle, NHLogger *i_logger);
static void
pv_close_log(std::ifstream &o_file);

static void
pv_log(NHLogLevel level, const char *msg, void *user)
{
    (void)(user);
    printf("%d: %s\n", level, msg);
}

#define MY_LOG(i_logger, i_level, ...)                                         \
    if ((i_logger) && (i_logger)->active >= (i_level))                         \
    {                                                                          \
        std::string s = fmt::format(__VA_ARGS__);                              \
        (i_logger)->log((i_level), s.c_str(), (i_logger)->user);               \
    }
#define MY_LOG_INFO(i_logger, ...) MY_LOG(i_logger, NH_LOG_INFO, __VA_ARGS__)
#define MY_LOG_ERROR(i_logger, ...) MY_LOG(i_logger, NH_LOG_ERROR, __VA_ARGS__)
#define MY_LOG_TRACE(i_logger, ...) MY_LOG(i_logger, NH_LOG_TRACE, __VA_ARGS__)

class cpu_test : public ::testing::Test {
  protected:
    void
    SetUp() override
    {
        console = NH_NULL;
    }

    void
    TearDown() override
    {
        if (NH_VALID(console))
        {
            nh_release_console(console);
        }
    }

    NHConsole console;
};

TEST_F(cpu_test, nestest)
{
    NHLogger logger{pv_log, nullptr, NH_LOG_TRACE};
    console = nh_new_console(&logger);
    ASSERT_TRUE(NH_VALID(console));
    auto rom_path = nb::resolve_exe_dir("nestest.nes");
    ASSERT_FALSE(NH_FAILED(nh_insert_cartridge(console, rom_path.c_str())));
    nh_power_up(console);

    bool open = false;
    auto log_file = pv_open_log(nb::resolve_exe_dir("nestest.log"), open);
    ASSERT_TRUE(open);

    NHCPU cpu = nh_test_get_cpu(console);
    constexpr NHAddr ENTRY = 0xC000;
    nh_test_cpu_set_entry(cpu, ENTRY);
    nh_test_cpu_set_p(cpu, 0x24);

    bool init_log = true;
    std::unordered_set<NHByte> opcode_set;
    std::size_t i_instr = 0;
    while (i_instr < 8990)
    {
        auto cmp_log = [&]() {
            constexpr NHCycle CYC_BASE = 7;
            // log the process
            NHByte bytes[3];
            int byte_count;
            nh_test_cpu_instr_bytes(cpu, nh_test_cpu_pc(cpu), bytes,
                                    &byte_count);
            MY_LOG_TRACE(
                &logger, "Instruction {:>4}: ${:04X} {:02X} {} {}", i_instr + 1,
                nh_test_cpu_pc(cpu), bytes[0],
                byte_count >= 2 ? fmt::format("{:02X}", bytes[1]) : "  ",
                byte_count >= 3 ? fmt::format("{:02X}", bytes[2]) : "  ");
            MY_LOG_TRACE(&logger,
                         "A:{:02X} X:{:02X} Y:{:02X} "
                         "P:{:02X} SP:{:02X}",
                         nh_test_cpu_a(cpu), nh_test_cpu_x(cpu),
                         nh_test_cpu_y(cpu), nh_test_cpu_p(cpu),
                         nh_test_cpu_s(cpu));
            MY_LOG_TRACE(&logger, "CYC: {}", CYC_BASE + nh_test_cpu_cycle(cpu));
            // log opcode coverage
            opcode_set.insert(bytes[0]);

            if (!pv_compare_log_line(&log_file, cpu, i_instr, CYC_BASE,
                                     &logger))
            {
                ASSERT_TRUE(false);
            }
        };

        if (init_log)
        {
            cmp_log();
            init_log = false;
        }
        int cpu_instr_done = false;
        nh_tick(console, &cpu_instr_done);
        if (cpu_instr_done)
        {
            ++i_instr;
            cmp_log();
        }
    }

    MY_LOG_INFO(&logger, "Opcode kinds covered: {}", opcode_set.size());
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
pv_compare_log_line(std::ifstream *io_file, NHCPU i_cpu, std::size_t i_instr,
                    NHCycle i_base_cycle, NHLogger *i_logger)
{
    std::string line;
    std::getline(*io_file, line);
    if (line.empty())
    {
        MY_LOG_ERROR(i_logger, "Empty log line");
        return false;
    }

    // instruction bytes
    {
        NHByte bytes[3];
        int byte_count;
        nh_test_cpu_instr_bytes(i_cpu, nh_test_cpu_pc(i_cpu), bytes,
                                &byte_count);
        std::string actual = fmt::format(
            "{:04X}  {:02X} {} {}", nh_test_cpu_pc(i_cpu), bytes[0],
            byte_count >= 2 ? fmt::format("{:02X}", bytes[1]) : "  ",
            byte_count >= 3 ? fmt::format("{:02X}", bytes[2]) : "  ");
        // [0, 14)
        std::string expected = line.substr(0, 14);

        if (actual != expected)
        {
            MY_LOG_ERROR(i_logger, "Diff failed at Instruction {}: [{}]",
                         i_instr + 1, actual);
            return false;
        }
    }
    // register state
    {
        std::string actual = fmt::format(
            "A:{:02X} X:{:02X} Y:{:02X} "
            "P:{:02X} SP:{:02X}",
            nh_test_cpu_a(i_cpu), nh_test_cpu_x(i_cpu), nh_test_cpu_y(i_cpu),
            nh_test_cpu_p(i_cpu), nh_test_cpu_s(i_cpu));
        // [48, 73)
        std::string expected = line.substr(48, 73 - 48);

        if (actual != expected)
        {
            MY_LOG_ERROR(i_logger, "Diff failed at Instruction {}: [{}]",
                         i_instr + 1, actual);
            return false;
        }
    }
    // cycle
    {
        std::string actual =
            std::to_string(i_base_cycle + nh_test_cpu_cycle(i_cpu));
        std::string expected = line.substr(line.rfind("CYC:") + 4);
        expected.pop_back(); // '\r'
        if (actual != expected)
        {
            MY_LOG_ERROR(i_logger, "Diff failed at Instruction {}: [{}] cycles",
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
