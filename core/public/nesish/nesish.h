#pragma once

#include "dllexport.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define NH_NULL nullptr
#else
#define NH_NULL 0
#endif
#define NH_VALID(handle) ((handle) != NH_NULL)

typedef unsigned char NHByte;  // At least 8-bit by both standards
typedef unsigned short NHAddr; // At least 16-bit by both standards
typedef size_t NHCycle;

#define NH_NES_WIDTH 256
#define NH_NES_HEIGHT 240

typedef enum {
    NH_ERR_OK = 0,
    NH_ERR_UNINITIALIZED,    // Uninitialized.
    NH_ERR_INVALID_ARGUMENT, // Invalid argument.
    NH_ERR_CORRUPTED,        // Invalid or corrupted resource.
    NH_ERR_UNAVAILABLE,      // Requested resource is unavailable.
    NH_ERR_UNIMPLEMENTED,    // TODO marker, shouldn't be exposed to production
                             // code.
    NH_ERR_PROGRAMMING,      // Someone wrote a bug, an unexpected behavior.
    NH_ERR_READ_ONLY,        // Attempted to write to read only area.
    NH_ERR_WRITE_ONLY,       // Attempted to read from write only area.
} NHErr;

#define NH_FAILED(err) ((err) != NH_ERR_OK)

/// @note Order matters for implementation
typedef enum {
    NH_LOG_OFF,
    NH_LOG_FATAL,
    NH_LOG_ERROR,
    NH_LOG_WARN,
    NH_LOG_INFO,
    NH_LOG_DEBUG,
    NH_LOG_TRACE,
} NHLogLevel;

typedef struct NHLogger {
    void (*log)(NHLogLevel level, const char *msg, void *user);
    void *user;
    NHLogLevel active;
} NHLogger;

typedef struct NHConsoleTy *NHConsole;

NH_API NHConsole
nh_new_console(NHLogger *logger);
NH_API void
nh_release_console(NHConsole console);

/// @note Values used as indices in implementation
typedef enum {
    NH_CTRL_P1 = 0,
    NH_CTRL_P2 = 1,
} NHCtrlSlot;

typedef enum {
    NH_KEY_A = 0,
    NH_KEY_B = 1,
    NH_KEY_SELECT = 2,
    NH_KEY_START = 3,
    NH_KEY_UP = 4,
    NH_KEY_DOWN = 5,
    NH_KEY_LEFT = 6,
    NH_KEY_RIGHT = 7,
} NHKeyIndex;

#define NH_KEYS 8
#define NH_KEY_BEGIN NH_KEY_A
#define NH_KEY_END (NH_KEY_RIGHT + 1)

typedef struct NHController {
    void (*strobe)(int enabled, void *user);
    int (*report)(void *user);
    void (*reset)(void *user);
    void *user;
} NHController;

NH_API void
nh_plug_ctrl(NHConsole console, NHCtrlSlot slot, NHController *ctrl);
NH_API void
nh_unplug_ctrl(NHConsole console, NHCtrlSlot slot);

NH_API NHErr
nh_insert_cart(NHConsole console, const char *rom_path);

NH_API void
nh_power_up(NHConsole console);
NH_API void
nh_reset(NHConsole console);

NH_API NHCycle
nh_advance(NHConsole console, double delta);
NH_API int
nh_tick(NHConsole console, int *cpu_instr);
NH_API double
nh_elapsed(NHConsole console, NHCycle ticks);

typedef struct NHFrameTy *NHFrame;

NH_API int
nh_frm_width(NHFrame frame);
NH_API int
nh_frm_height(NHFrame frame);
NH_API const NHByte *
nh_frm_data(NHFrame frame);

NH_API NHFrame
nh_get_frm(NHConsole console);

NH_API float
nh_get_sample_rate(NHConsole console);
NH_API float
nh_get_sample(NHConsole console);

typedef enum {
    NHD_DBG_OFF = 0,
    NHD_DBG_PALETTE = 1 << 0,
    NHD_DBG_OAM = 1 << 1,
    NHD_DBG_PATTERN = 1 << 2,
} NHDFlags;

NH_API void
nhd_turn_debug_on(NHConsole console, NHDFlags flag);
NH_API void
nhd_turn_debug_off(NHConsole console, NHDFlags flag);

typedef struct NHDPatternTableTy *NHDPatternTable;

NH_API int
nhd_ptn_table_width(NHDPatternTable ptn_table);
NH_API int
nhd_ptn_table_height(NHDPatternTable ptn_table);
NH_API const NHByte *
nhd_ptn_table_data(NHDPatternTable ptn_table);

NH_API int
nhd_ptn_table_tiles_width(NHDPatternTable ptn_table);
NH_API int
nhd_ptn_table_tiles_height(NHDPatternTable ptn_table);
NH_API int
nhd_ptn_table_tile_width(NHDPatternTable ptn_table);
NH_API int
nhd_ptn_table_tile_height(NHDPatternTable ptn_table);

NH_API NHDPatternTable
nhd_get_ptn_table(NHConsole console, int right);

typedef enum {
    NHD_PALETTE_BG0 = 0,
    NHD_PALETTE_BG1 = 1,
    NHD_PALETTE_BG2 = 2,
    NHD_PALETTE_BG3 = 3,
    NHD_PALETTE_SP0 = 4,
    NHD_PALETTE_SP1 = 5,
    NHD_PALETTE_SP2 = 6,
    NHD_PALETTE_SP3 = 7,
} NHDPaletteSet;

NH_API void
nhd_set_ptn_table_palette(NHConsole console, NHDPaletteSet palette);

typedef struct NHDOAMTy *NHDOAM;
typedef struct NHDSpriteTy *NHDSprite;

#define NHD_OAM_SPRITES 64
NH_API NHDSprite
nhd_oam_sprite(NHDOAM oam, int index);

NH_API
int
nhd_sprite_width(NHDSprite sprite);
NH_API
int
nhd_sprite_height(NHDSprite sprite);
NH_API
const NHByte *
nhd_sprite_data(NHDSprite sprite);

NH_API NHByte
nhd_sprite_y(NHDSprite sprite);
NH_API NHByte
nhd_sprite_tile(NHDSprite sprite);
NH_API NHByte
nhd_sprite_attr(NHDSprite sprite);
NH_API NHByte
nhd_sprite_x(NHDSprite sprite);

NH_API int
nhd_sprite_palette_set(NHDSprite sprite);
NH_API int
nhd_sprite_background(NHDSprite sprite);
NH_API int
nhd_sprite_flip_x(NHDSprite sprite);
NH_API int
nhd_sprite_flip_y(NHDSprite sprite);

NH_API NHDOAM
nhd_get_oam(NHConsole console);

typedef struct NHDColor {
    NHByte r, g, b;
    NHByte index;
} NHDColor;

typedef struct NHDPaletteTy *NHDPalette;

#define NHD_PALETTE_COLORS 32
NH_API NHDColor
nhd_palette_color(NHDPalette palette, int index);

NH_API NHDPalette
nhd_get_palette(NHConsole console);

typedef struct NHCPUTy *NHCPU;

NH_API NHCPU
nh_test_get_cpu(NHConsole console);

NH_API void
nh_test_cpu_set_entry(NHCPU cpu, NHAddr entry);
NH_API void
nh_test_cpu_set_p(NHCPU cpu, NHByte val);

NH_API NHAddr
nh_test_cpu_pc(NHCPU cpu);
NH_API NHByte
nh_test_cpu_a(NHCPU cpu);
NH_API NHByte
nh_test_cpu_x(NHCPU cpu);
NH_API NHByte
nh_test_cpu_y(NHCPU cpu);
NH_API NHByte
nh_test_cpu_p(NHCPU cpu);
NH_API NHByte
nh_test_cpu_s(NHCPU cpu);
NH_API NHCycle
nh_test_cpu_cycle(NHCPU cpu);
NH_API void
nh_test_cpu_instr_bytes(NHCPU cpu, NHAddr addr, NHByte bytes[3], int *size);

#ifdef __cplusplus
}
#endif
