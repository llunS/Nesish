#include "logger.hpp"

#include <string>
#include <utility>
#include <stack>

#include "nhbase/path.hpp"
#include "nhbase/filesystem.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "misc/exception.hpp"

#include <unordered_map>

namespace sh {

#ifndef SH_TGT_WEB
static std::string
pv_log_file_rel_exec_path();
static std::string
pv_backup_previous_logs(const std::string &i_log_exec_rel_path, int i_max_logs,
                        Logger *i_logger);
#endif

static const std::unordered_map<NHLogLevel, const char *> g_level_to_name{
    /* clang-format off */
    {NH_LOG_OFF, "Off"},
    {NH_LOG_FATAL, "Fatal"},
    {NH_LOG_ERROR, "Error"},
    {NH_LOG_WARN, "Warn"},
    {NH_LOG_INFO, "Info"},
    {NH_LOG_DEBUG, "Debug"},
    {NH_LOG_TRACE, "Trace"},
    /* clang-format on */
};

const char *
log_level_to_name(NHLogLevel i_level)
{
    auto it = g_level_to_name.find(i_level);
    return it != g_level_to_name.cend() ? it->second : "";
}

Logger::Logger(NHLogLevel i_level)
    : logger(nullptr)
    , level(NH_LOG_OFF)
{
    /* Create logger with stdout sink */
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    auto spd_logger = new spdlog::logger("Nesish", stdout_sink);
    this->logger = spd_logger;

    // Prompt log level
    spd_logger->set_level(spdlog::level::info);
    SH_LOG_INFO(this, "Log level: {}", log_level_to_name(i_level));
    // Set both levels
    set_level(i_level);

#ifndef SH_TGT_WEB
    /* Create file sink */
    constexpr int MAX_LOGS = 5;
    auto log_filepath =
        pv_backup_previous_logs(pv_log_file_rel_exec_path(), MAX_LOGS, this);
    if (!log_filepath.empty())
    {
        SH_TRY
        {
            auto file_sink =
                std::make_shared<spdlog::sinks::basic_file_sink_st>(
                    log_filepath, true);
            spd_logger->sinks().push_back(std::move(file_sink));
            SH_LOG_INFO(this, "Log file: {}", log_filepath);
        }
        SH_CATCH(const spdlog::spdlog_ex &e)
        {
            SH_LOG_ERROR(this, "Failed to create log file: {}, {}",
                         log_filepath, e.what());
        }
    }
    else
    {
        SH_LOG_ERROR(this, "Failed to create log file: {}",
                     std::string("./") + pv_log_file_rel_exec_path());
    }
#endif
}

void
Logger::set_level(NHLogLevel i_level)
{
    this->level = i_level;
    switch (i_level)
    {
        case NH_LOG_FATAL:
            this->logger->set_level(spdlog::level::critical);
            break;
        case NH_LOG_ERROR:
            this->logger->set_level(spdlog::level::err);
            break;
        case NH_LOG_WARN:
            this->logger->set_level(spdlog::level::warn);
            break;
        case NH_LOG_INFO:
            this->logger->set_level(spdlog::level::info);
            break;
        case NH_LOG_DEBUG:
            this->logger->set_level(spdlog::level::debug);
            break;
        case NH_LOG_TRACE:
            this->logger->set_level(spdlog::level::trace);
            break;

        // Invalid
        default:
            this->logger->set_level(spdlog::level::off);
            this->level = NH_LOG_OFF;
            break;
    }
}

Logger::~Logger()
{
    if (logger)
    {
        delete logger;
    }
}

#ifndef SH_TGT_WEB

std::string
pv_log_file_rel_exec_path()
{
    return nb::path_join("logs", "log.txt");
}

std::string
pv_backup_previous_logs(const std::string &i_log_exec_rel_path, int i_max_logs,
                        Logger *i_logger)
{
    // sanity checks.
    if (i_log_exec_rel_path.empty())
    {
        return "";
    }
    if (i_max_logs <= 0)
    {
        return "";
    }

    // collect all rename items,
    // excluding the last one ([i_max_logs - 1]).
    std::stack<int> rename_indices;
    int curr = 0;
    while (curr < i_max_logs - 1)
    {
        auto basename = curr == 0
                            ? i_log_exec_rel_path
                            : i_log_exec_rel_path + "." + std::to_string(curr);
        std::string cur_abs_path = nb::resolve_exe_dir(basename);
        if (nb::file_exists(cur_abs_path))
        {
            rename_indices.push(curr);

            ++curr;
        }
        else
        {
            break;
        }
    }

    // rename all files (+1) if needed, processing backwards.
    while (!rename_indices.empty())
    {
        int i = rename_indices.top();
        rename_indices.pop();

        auto basename = i == 0 ? i_log_exec_rel_path
                               : i_log_exec_rel_path + "." + std::to_string(i);
        std::string cur_abs_path = nb::resolve_exe_dir(basename);

        // something may happen since we collect it, we not exist anymore,
        // ignore it.
        if (!nb::file_exists(cur_abs_path))
        {
            continue;
        }

        auto next_basename = i_log_exec_rel_path + "." + std::to_string(i + 1);
        auto next_file_abs_path = nb::resolve_exe_dir(next_basename);
        if (!nb::file_rename(cur_abs_path, next_file_abs_path, true))
        {
            // if we failed to rename, then this file may be overwritten.
            // this is undesirable for all previously log files except the one
            // at the end ([i_max_logs - 1]).

            // this is certainly an error, but we don't want to abort the whole
            // process because of it, so we‘d just log the error and proceed.
            SH_LOG_WARN(i_logger, "Previous log file may be overwritten: {}",
                        cur_abs_path);
        }
        else
        {
            SH_LOG_TRACE(i_logger,
                         "Previous log file has been moved forward: {}",
                         cur_abs_path);
        }
    }

    return nb::resolve_exe_dir(i_log_exec_rel_path);
}

#endif

} // namespace sh
