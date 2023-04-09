#include "config.hpp"

#include <unordered_map>
#include <string>

#include "lua.hpp"
#include "glfw/glfw3.h"

#include "nesish/nesish.h"
#include "nhbase/path.hpp"
#include "nhbase/filesystem.hpp"

namespace sh {

static std::string
pv_init_key_cfg(Logger *i_logger);

static void
pv_setup_key_env(lua_State *L);
static void
pv_setup_key_def(lua_State *L, const char *i_vkey_name, VirtualKey i_vkey);

static bool
pv_load_port_cfg(lua_State *L, const char *i_portname, KeyMapping &o_mapping,
                 Logger *i_logger);

bool
load_key_config(KeyMapping &o_p1, KeyMapping &o_p2, Logger *i_logger)
{
    std::string input_cfg = pv_init_key_cfg(i_logger);
    if (input_cfg.empty())
    {
        return false;
    }
    SH_LOG_INFO(i_logger, "Using input config: {}", input_cfg);

    bool ok = true;

    lua_State *L = luaL_newstate();
    pv_setup_key_env(L);

    int error = luaL_loadfile(L, input_cfg.c_str()) || lua_pcall(L, 0, 0, 0);
    if (error)
    {
        SH_LOG_ERROR(i_logger, "{}", lua_tostring(L, -1));
        lua_pop(L, 1); /* pop error message from the stack */
        ok = false;
        goto l_end;
    }

    if (!pv_load_port_cfg(L, "P1", o_p1, i_logger))
    {
        ok = false;
        goto l_end;
    }
    if (!pv_load_port_cfg(L, "P2", o_p2, i_logger))
    {
        ok = false;
        goto l_end;
    }

l_end:
    lua_close(L);
    return ok;
}

std::string
pv_init_key_cfg(Logger *i_logger)
{
    std::string input_cfg = nb::path_join_exe("config/input.lua");
    if (nb::file_exists(input_cfg))
    {
        return input_cfg;
    }
    std::string default_cfg = nb::path_join_exe("config/input_default.lua");
    if (!nb::file_exists(default_cfg))
    {
        SH_LOG_ERROR(i_logger, "Default input config missing: {}", default_cfg);
        return "";
    }
    if (nb::file_copy(default_cfg, input_cfg))
    {
        return input_cfg;
    }
    return "";
}

void
pv_setup_key_env(lua_State *L)
{
#define STRINGIFY(x) #x
#define REG_KEY(i_keyname)                                                     \
    pv_setup_key_def(L, STRINGIFY(KEY_##i_keyname), GLFW_KEY_##i_keyname)

    /* Printable keys */
    REG_KEY(SPACE);
    REG_KEY(APOSTROPHE);
    REG_KEY(COMMA);
    REG_KEY(MINUS);
    REG_KEY(PERIOD);
    REG_KEY(SLASH);
    REG_KEY(0);
    REG_KEY(1);
    REG_KEY(2);
    REG_KEY(3);
    REG_KEY(4);
    REG_KEY(5);
    REG_KEY(6);
    REG_KEY(7);
    REG_KEY(8);
    REG_KEY(9);
    REG_KEY(SEMICOLON);
    REG_KEY(EQUAL);
    REG_KEY(A);
    REG_KEY(B);
    REG_KEY(C);
    REG_KEY(D);
    REG_KEY(E);
    REG_KEY(F);
    REG_KEY(G);
    REG_KEY(H);
    REG_KEY(I);
    REG_KEY(J);
    REG_KEY(K);
    REG_KEY(L);
    REG_KEY(M);
    REG_KEY(N);
    REG_KEY(O);
    REG_KEY(P);
    REG_KEY(Q);
    REG_KEY(R);
    REG_KEY(S);
    REG_KEY(T);
    REG_KEY(U);
    REG_KEY(V);
    REG_KEY(W);
    REG_KEY(X);
    REG_KEY(Y);
    REG_KEY(Z);
    REG_KEY(LEFT_BRACKET);
    REG_KEY(BACKSLASH);
    REG_KEY(RIGHT_BRACKET);
    REG_KEY(GRAVE_ACCENT);

    /* Function keys */
    REG_KEY(ESCAPE);
    REG_KEY(ENTER);
    REG_KEY(TAB);
    REG_KEY(BACKSPACE);
    REG_KEY(INSERT);
    REG_KEY(DELETE);
    REG_KEY(RIGHT);
    REG_KEY(LEFT);
    REG_KEY(DOWN);
    REG_KEY(UP);
    REG_KEY(PAGE_UP);
    REG_KEY(PAGE_DOWN);
    REG_KEY(HOME);
    REG_KEY(END);
    REG_KEY(CAPS_LOCK);
    REG_KEY(SCROLL_LOCK);
    REG_KEY(NUM_LOCK);
    REG_KEY(PRINT_SCREEN);
    REG_KEY(PAUSE);
    REG_KEY(F1);
    REG_KEY(F2);
    REG_KEY(F3);
    REG_KEY(F4);
    REG_KEY(F5);
    REG_KEY(F6);
    REG_KEY(F7);
    REG_KEY(F8);
    REG_KEY(F9);
    REG_KEY(F10);
    REG_KEY(F11);
    REG_KEY(F12);
    REG_KEY(F13);
    REG_KEY(F14);
    REG_KEY(F15);
    REG_KEY(F16);
    REG_KEY(F17);
    REG_KEY(F18);
    REG_KEY(F19);
    REG_KEY(F20);
    REG_KEY(F21);
    REG_KEY(F22);
    REG_KEY(F23);
    REG_KEY(F24);
    REG_KEY(F25);
    REG_KEY(KP_0);
    REG_KEY(KP_1);
    REG_KEY(KP_2);
    REG_KEY(KP_3);
    REG_KEY(KP_4);
    REG_KEY(KP_5);
    REG_KEY(KP_6);
    REG_KEY(KP_7);
    REG_KEY(KP_8);
    REG_KEY(KP_9);
    REG_KEY(KP_DECIMAL);
    REG_KEY(KP_DIVIDE);
    REG_KEY(KP_MULTIPLY);
    REG_KEY(KP_SUBTRACT);
    REG_KEY(KP_ADD);
    REG_KEY(KP_ENTER);
    REG_KEY(KP_EQUAL);
    REG_KEY(LEFT_SHIFT);
    REG_KEY(LEFT_CONTROL);
    REG_KEY(LEFT_ALT);
    REG_KEY(LEFT_SUPER);
    REG_KEY(RIGHT_SHIFT);
    REG_KEY(RIGHT_CONTROL);
    REG_KEY(RIGHT_ALT);
    REG_KEY(RIGHT_SUPER);
    REG_KEY(MENU);

#undef REG_KEY
#undef STRINGIFY
}

void
pv_setup_key_def(lua_State *L, const char *i_vkey_name, VirtualKey i_vkey)
{
    lua_pushinteger(L, i_vkey);
    lua_setglobal(L, i_vkey_name);
}

bool
pv_load_port_cfg(lua_State *L, const char *i_portname, KeyMapping &o_mapping,
                 Logger *i_logger)
{
    lua_getglobal(L, i_portname);
    if (lua_istable(L, -1))
    {
        auto get1key = [i_portname, i_logger](lua_State *L,
                                              const char *i_nhkey_name,
                                              VirtualKey &o_vkey) -> bool {
            int ok = true;

            lua_getfield(L, -1, i_nhkey_name);
            if (!lua_isnumber(L, -1))
            {
                SH_LOG_ERROR(i_logger, "{}.{}: Key invalid or missing",
                             i_portname, i_nhkey_name);
                ok = false;
                goto l_end;
            }
            o_vkey = (VirtualKey)lua_tointeger(L, -1);

        l_end:
            lua_pop(L, 1);
            return ok;
        };

        KeyMapping mapping{};
        static std::unordered_map<NHKey, const char *> NHKEY_NAMES{
            {NH_KEY_A, "A"},           {NH_KEY_B, "B"},
            {NH_KEY_SELECT, "SELECT"}, {NH_KEY_START, "START"},
            {NH_KEY_UP, "UP"},         {NH_KEY_DOWN, "DOWN"},
            {NH_KEY_LEFT, "LEFT"},     {NH_KEY_RIGHT, "RIGHT"},
        };
        for (const auto &pair : NHKEY_NAMES)
        {
            VirtualKey key;
            if (!get1key(L, pair.second, key))
            {
                return false;
            }
            mapping[pair.first] = key;
        }
        o_mapping = mapping;
        return true;
    }
    else
    {
        SH_LOG_ERROR(i_logger, "{}: Config does not exist or is not a table",
                     i_portname);
        return false;
    }
}

} // namespace sh
