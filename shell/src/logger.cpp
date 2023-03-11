#include "logger.hpp"

#include <string>
#include <utility>
#include <stack>

#include "nhbase/path.hpp"
#include "nhbase/filesystem.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#define MAX_LOGS 5

namespace sh {

static spdlog::logger *g_logger;
static NHLogLevel g_log_level = SH_DEFAULT_LOG_LEVEL;

static std::string
pv_log_file_rel_exec_path();
static std::string
pv_backup_previous_logs(const std::string &i_log_exec_rel_path, int i_max_logs,
                        spdlog::logger *i_logger);

void
init_logger(NHLogLevel i_level)
{
    if (g_logger)
    {
        return;
    }

    g_log_level = i_level;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    auto logger = new spdlog::logger("Nesish", console_sink);

    switch (i_level)
    {
        case NH_LOG_FATAL:
            logger->set_level(spdlog::level::critical);
            break;
        case NH_LOG_ERROR:
            logger->set_level(spdlog::level::err);
            break;
        case NH_LOG_WARN:
            logger->set_level(spdlog::level::warn);
            break;
        case NH_LOG_INFO:
            logger->set_level(spdlog::level::info);
            break;
        case NH_LOG_DEBUG:
            logger->set_level(spdlog::level::debug);
            break;
        case NH_LOG_TRACE:
            logger->set_level(spdlog::level::trace);
            break;

        default:
            logger->set_level(spdlog::level::off);
            break;
    }

    auto log_filepath =
        pv_backup_previous_logs(pv_log_file_rel_exec_path(), MAX_LOGS, logger);
    if (!log_filepath.empty())
    {
        try
        {
            auto file_sink =
                std::make_shared<spdlog::sinks::basic_file_sink_st>(
                    log_filepath, true);
            logger->sinks().push_back(std::move(file_sink));
            SH_LOG_INFO(logger, "Log file at: {}", log_filepath);
        }
        catch (const spdlog::spdlog_ex &e)
        {
            SH_LOG_ERROR(logger, "Failed to create log file: {}, {}",
                         log_filepath, e.what());
        }
    }
    else
    {
        SH_LOG_ERROR(logger, "Failed to create log file: {}",
                     std::string("./") + pv_log_file_rel_exec_path());
    }

    g_logger = logger;
}

spdlog::logger *
get_logger()
{
    if (!g_logger)
    {
        init_logger(SH_DEFAULT_LOG_LEVEL);
    }

    return g_logger;
}

NHLogLevel
get_log_level()
{
    return g_log_level;
}

std::string
pv_log_file_rel_exec_path()
{
    return nb::path_join("logs", "log.txt");
}

std::string
pv_backup_previous_logs(const std::string &i_log_exec_rel_path, int i_max_logs,
                        spdlog::logger *i_logger)
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
        std::string cur_abs_path = nb::path_join_exe(basename);
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
        std::string cur_abs_path = nb::path_join_exe(basename);

        // something may happen since we collect it, we not exist anymore,
        // ignore it.
        if (!nb::file_exists(cur_abs_path))
        {
            continue;
        }

        auto next_basename = i_log_exec_rel_path + "." + std::to_string(i + 1);
        auto next_file_abs_path = nb::path_join_exe(next_basename);
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

    return nb::path_join_exe(i_log_exec_rel_path);
}

} // namespace sh
