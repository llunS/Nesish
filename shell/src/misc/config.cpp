#include "misc/config.hpp"

#include "nhbase/path.hpp"
#include "nhbase/filesystem.hpp"
#include "nhbase/vc_intrinsics.hpp"

#include <string>
#include <cstdio>

namespace sh {

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
    return !err;
}

bool
reset_default_key_config(KeyMapping *o_p1, KeyMapping *o_p2, Logger *i_logger)
{
    KeyMapping p1{}, p2{};
    std::string input_cfg = nb::path_join_exe("config/input.ini");
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
    }
    return !err;
}

std::string
pv_get_or_init_key_cfg(Logger *i_logger)
{
    // Check if exists
    std::string input_cfg = nb::path_join_exe("config/input.ini");
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

} // namespace sh
