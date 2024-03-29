#include "misc/config.hpp"

#include "nhbase/path.hpp"
#include "nhbase/filesystem.hpp"
#include "nhbase/vc_intrinsics.hpp"
#include "misc/exception.hpp"

NB_VC_WARNING_PUSH
NB_VC_WARNING_DISABLE(4706)
#include "mini/ini.h"
NB_VC_WARNING_POP

#include <string>
#include <cstdio>

#ifdef SH_TGT_WEB
#include "misc/web_utils.hpp"
#endif

namespace sh {

#ifndef SH_TGT_WEB
#define CONFIG_PREFIX_DIR "config/"
#else
#define CONFIG_PREFIX_DIR ""
#endif

static constexpr VirtualKey g_key_config[NH_KEYS * 2] = {
    /* P1 */
    GLFW_KEY_K, // A
    GLFW_KEY_J, // B
    GLFW_KEY_V, // SELECT
    GLFW_KEY_B, // START
    GLFW_KEY_W, // UP
    GLFW_KEY_S, // DOWN
    GLFW_KEY_A, // LEFT
    GLFW_KEY_D, // RIGHT

    /* P2 */
    GLFW_KEY_PERIOD,
    GLFW_KEY_COMMA,
    GLFW_KEY_SEMICOLON,
    GLFW_KEY_APOSTROPHE,
    GLFW_KEY_UP,
    GLFW_KEY_DOWN,
    GLFW_KEY_LEFT,
    GLFW_KEY_RIGHT,
};
static_assert(g_key_config[NH_KEYS * 2 - 1] != 0, "Missing elements");

static std::string
pv_get_or_init_key_cfg(Logger *i_logger);
static bool
pv_load_key_cfg(const std::string &i_config_file, KeyMapping &o_p1,
                KeyMapping &o_p2, Logger *i_logger);

bool
load_key_config(KeyMapping &o_p1, KeyMapping &o_p2, Logger *i_logger)
{
    std::string input_cfg = pv_get_or_init_key_cfg(i_logger);
    if (input_cfg.empty())
    {
        return false;
    }

    if (!pv_load_key_cfg(input_cfg, o_p1, o_p2, i_logger))
    {
        return false;
    }

    SH_LOG_INFO(i_logger, "Using input config: {}", input_cfg);
    return true;
}

bool
save_key_config(NHCtrlPort i_port, NHKey i_key, VirtualKey i_vkey,
                Logger *i_logger)
{
    std::string input_cfg = pv_get_or_init_key_cfg(i_logger);
    if (!nb::file_exists(input_cfg))
    {
        SH_LOG_ERROR(i_logger, "Input config doesn't exist: {}", input_cfg);
        return false;
    }

    int err = 0;
    NB_VC_WARNING_PUSH
    NB_VC_WARNING_DISABLE(4996) // false positive
    std::FILE *fp = std::fopen(input_cfg.c_str(), "rb+");
    NB_VC_WARNING_POP
    // Seek to the line and modify the data
    {
        if (!fp)
        {
            SH_LOG_ERROR(i_logger, "Failed to open input config to save: {}",
                         input_cfg);
            err = 1;
            goto l_end;
        }
        constexpr long LINE_BYTES = sizeof(VirtualKey) + sizeof(char);
        long index = i_port * NH_KEYS + i_key;
        if (std::fseek(fp, index * LINE_BYTES, SEEK_SET))
        {
            SH_LOG_ERROR(i_logger, "Failed to seek to save position: {} {}",
                         input_cfg, index);
            err = 1;
            goto l_end;
        }
        auto written = std::fwrite(&i_vkey, sizeof(i_vkey), 1, fp);
        if (written < 1)
        {
            SH_LOG_ERROR(i_logger,
                         "Failed to save to input config: {} {} {} {}",
                         input_cfg, index, i_vkey, written);
            err = 1;
            goto l_end;
        }
    }

l_end:
    if (fp)
    {
        std::fclose(fp);
    }
    if (!err)
    {
#ifdef SH_TGT_WEB
        web_sync_fs_async();
#endif
    }
    return !err;
}

bool
reset_default_key_config(KeyMapping *o_p1, KeyMapping *o_p2, Logger *i_logger)
{
    KeyMapping p1{}, p2{};
    std::string input_cfg = nb::resolve_exe_dir(CONFIG_PREFIX_DIR "input.ini");
    std::FILE *fp = nullptr;
    int err = 0;
    {
        NB_VC_WARNING_PUSH
        NB_VC_WARNING_DISABLE(4996) // false positive
        fp = std::fopen(input_cfg.c_str(), "wb");
        NB_VC_WARNING_POP
        if (!fp)
        {
            SH_LOG_ERROR(i_logger, "Failed to open input config: {}",
                         input_cfg);
            err = 1;
            goto l_end;
        }
        for (NHKey i = 0; i < NH_KEYS * 2; ++i)
        {
            auto written =
                std::fwrite(&g_key_config[i], sizeof(g_key_config[i]), 1, fp);
            if (written < 1)
            {
                SH_LOG_ERROR(i_logger,
                             "Failed to write to input config: {} {} {} {}",
                             input_cfg, i, g_key_config[i], written);
                err = 1;
                goto l_end;
            }

            constexpr char lf = '\n';
            written = std::fwrite(&lf, sizeof(lf), 1, fp);
            if (written < 1)
            {
                SH_LOG_ERROR(i_logger,
                             "Failed to write LF to input config: {} {} {} {}",
                             input_cfg, i, g_key_config[i], written);
                err = 1;
                goto l_end;
            }

            if (i < NH_KEYS)
            {
                p1[i] = g_key_config[i];
            }
            else
            {
                p2[i - NH_KEYS] = g_key_config[i];
            }
        }
    }

l_end:
    if (fp)
    {
        std::fclose(fp);
    }
    if (!err)
    {
        if (o_p1)
        {
            *o_p1 = p1;
        }
        if (o_p2)
        {
            *o_p2 = p2;
        }
#ifdef SH_TGT_WEB
        web_sync_fs_async();
#endif
    }
    return !err;
}

std::string
pv_get_or_init_key_cfg(Logger *i_logger)
{
    // Check if exists
    std::string input_cfg = nb::resolve_exe_dir(CONFIG_PREFIX_DIR "input.ini");
    if (nb::file_exists(input_cfg))
    {
        return input_cfg;
    }

    // Write defaults to config file if not exist
    bool ok = reset_default_key_config(nullptr, nullptr, i_logger);
    return !ok ? "" : input_cfg;
}

bool
pv_load_key_cfg(const std::string &i_config_file, KeyMapping &o_p1,
                KeyMapping &o_p2, Logger *i_logger)
{
    if (!nb::file_exists(i_config_file))
    {
        SH_LOG_ERROR(i_logger, "Input config doesn't exist: {}", i_config_file);
        return false;
    }

    int err = 0;
    NB_VC_WARNING_PUSH
    NB_VC_WARNING_DISABLE(4996) // false positive
    std::FILE *fp = std::fopen(i_config_file.c_str(), "rb");
    NB_VC_WARNING_POP
    if (!fp)
    {
        SH_LOG_ERROR(i_logger, "Failed to read input config {}", i_config_file);
        err = 1;
        goto l_end;
    }
    for (NHKey i = 0; i < NH_KEYS * 2; ++i)
    {
        VirtualKey vkey;
        auto got = std::fread(&vkey, sizeof(vkey), 1, fp);
        if (got < 1)
        {
            SH_LOG_ERROR(i_logger, "Virtual key corrupted: {} {}", i, got);
            err = 1;
            goto l_end;
        }
        if (i < NH_KEYS)
        {
            o_p1[i] = vkey;
        }
        else
        {
            o_p2[i - NH_KEYS] = vkey;
        }
        char lf;
        got = std::fread(&lf, sizeof(lf), 1, fp);
        if (got < 1)
        {
            SH_LOG_ERROR(i_logger, "Failed to read LF: {} {}", i, got);
            err = 1;
            goto l_end;
        }
        if (lf != '\n')
        {
            SH_LOG_ERROR(i_logger, "Non-LF encountered: {} {:02X}", i, lf);
            err = 1;
            goto l_end;
        }
    }

l_end:
    if (fp)
    {
        std::fclose(fp);
    }
    return !err;
}

bool
load_single_bool(bool &o_val, const char *i_section, const char *i_key)
{
    SH_TRY
    {
        std::string user_cfg =
            nb::resolve_exe_dir(CONFIG_PREFIX_DIR "nesish.ini");
        if (!nb::file_exists(user_cfg))
        {
            return false;
        }

        mINI::INIFile inifile(user_cfg);
        mINI::INIStructure ini;
        if (!inifile.read(ini))
        {
            return false;
        }

        if (!ini.has(i_section))
        {
            return false;
        }
        if (!ini[i_section].has(i_key))
        {
            return false;
        }

        o_val = (ini[i_section][i_key] == "1");
        return true;
    }
    SH_CATCH(const std::exception &)
    {
        return false;
    }
}

bool
save_single_bool(bool i_val, const char *i_section, const char *i_key)
{
    SH_TRY
    {
        std::string user_cfg =
            nb::resolve_exe_dir(CONFIG_PREFIX_DIR "nesish.ini");

        mINI::INIFile inifile(user_cfg);
        mINI::INIStructure ini;
        // No-file returns false
        (void)inifile.read(ini);

        ini[i_section][i_key] = i_val ? "1" : "0";
        bool res = inifile.write(ini);
#ifdef SH_TGT_WEB
        web_sync_fs_async();
#endif
        return res;
    }
    SH_CATCH(const std::exception &)
    {
        return false;
    }
}

bool
load_log_level(NHLogLevel &o_val)
{
    SH_TRY
    {
        std::string user_cfg =
            nb::resolve_exe_dir(CONFIG_PREFIX_DIR "nesish.ini");
        if (!nb::file_exists(user_cfg))
        {
            return false;
        }

        mINI::INIFile inifile(user_cfg);
        mINI::INIStructure ini;
        if (!inifile.read(ini))
        {
            return false;
        }

        if (!ini.has("Debug"))
        {
            return false;
        }
        if (!ini["Debug"].has("LogLevel"))
        {
            return false;
        }
        auto val = static_cast<NHLogLevel>(std::stoi(ini["Debug"]["LogLevel"]));
        if (val < NH_LOG_OFF || val > NH_LOG_TRACE)
        {
            return false;
        }

        o_val = val;
        return true;
    }
    SH_CATCH(const std::exception &)
    {
        return false;
    }
}

bool
save_log_level(NHLogLevel i_val)
{
    SH_TRY
    {
        std::string user_cfg =
            nb::resolve_exe_dir(CONFIG_PREFIX_DIR "nesish.ini");

        mINI::INIFile inifile(user_cfg);
        mINI::INIStructure ini;
        // No-file returns false
        (void)inifile.read(ini);

        ini["Debug"]["LogLevel"] = std::to_string(i_val);
        bool res = inifile.write(ini);
#ifdef SH_TGT_WEB
        web_sync_fs_async();
#endif
        return res;
    }
    SH_CATCH(const std::exception &)
    {
        return false;
    }
}

} // namespace sh
