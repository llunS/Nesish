#include "nesish/nesish.h"

#include "console.h"

#include <stdlib.h>

#define NH_DECL_CONSOLE(handle) console_s *nh_console = (console_s *)(handle);
#define NH_DECL_CPU(handle) cpu_s *nh_cpu = (cpu_s *)(handle);
#define NH_DECL_FRM(handle)                                                    \
    const frmbuf_s *nh_frame = (const frmbuf_s *)(handle);

#define NHD_DECL_PTNTBL(handle)                                                \
    const dbgpattbl_s *nhd_ptn_table = (const dbgpattbl_s *)(handle);
#define NHD_DECL_OAM(handle)                                                   \
    const dbgoam_s *nhd_oam = (const dbgoam_s *)(handle);
#define NHD_DECL_SPRITE(handle)                                                \
    const dbgspr_s *nhd_sprite = (const dbgspr_s *)(handle);
#define NHD_DECL_PALETTE(handle)                                               \
    const dbgpal_s *nhd_palette = (const dbgpal_s *)(handle);

NHConsole
nh_new_console(NHLogger *logger)
{
    console_s *nh_console = malloc(sizeof(console_s));
    if (nh_console)
    {
        if (!console_Init(nh_console, logger))
        {
            free(nh_console);
            return NULL;
        }
    }
    return (NHConsole)nh_console;
}

void
nh_release_console(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    if (nh_console)
    {
        console_Deinit(nh_console);
        free(nh_console);
    }
}

void
nh_plug_ctrl(NHConsole console, NHCtrlPort slot, NHController *ctrl)
{
    NH_DECL_CONSOLE(console);
    console_PlugCtrl(nh_console, slot, ctrl);
}

void
nh_unplug_ctrl(NHConsole console, NHCtrlPort slot)
{
    NH_DECL_CONSOLE(console);
    console_UnplugCtrl(nh_console, slot);
}

NHErr
nh_insert_cartridge(NHConsole console, const char *rom_path)
{
    NH_DECL_CONSOLE(console);
    return console_InsertCart(nh_console, rom_path);
}

void
nh_remove_cartridge(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    console_RemoveCart(nh_console);
}

void
nh_power_up(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    console_Powerup(nh_console);
}

void
nh_reset(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    console_Reset(nh_console);
}

NHCycle
nh_advance(NHConsole console, double delta)
{
    NH_DECL_CONSOLE(console);
    return console_Advance(nh_console, delta);
}

int
nh_tick(NHConsole console, int *cpu_instr)
{
    NH_DECL_CONSOLE(console);
    bool b1;
    bool b0 = console_Tick(nh_console, &b1);
    if (cpu_instr)
    {
        *cpu_instr = b1;
    }
    return b0;
}

int
nh_frm_width(NHFrame frame)
{
    (void)(frame);
    return FRMBUF_WIDTH;
}
int
nh_frm_height(NHFrame frame)
{
    (void)(frame);
    return FRMBUF_HEIGHT;
}
const NHByte *
nh_frm_data(NHFrame frame)
{
    NH_DECL_FRM(frame);
    return frmbuf_Data(nh_frame);
}

NHFrame
nh_get_frm(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHFrame)console_GetFrm(nh_console);
}

int
nh_get_sample_rate(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return console_GetSampleRate(nh_console);
}
double
nh_get_sample(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return console_GetSample(nh_console);
}

void
nhd_turn_debug_on(NHConsole console, NHDFlag flag)
{
    NH_DECL_CONSOLE(console);
    console_SetDebugOn(nh_console, flag);
}
void
nhd_turn_debug_off(NHConsole console, NHDFlag flag)
{
    NH_DECL_CONSOLE(console);
    console_SetDebugOff(nh_console, flag);
}

int
nhd_ptn_table_width(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return dbgpattble_GetWidth(nhd_ptn_table);
}
int
nhd_ptn_table_height(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return dbgpattble_GetHeight(nhd_ptn_table);
}
const NHByte *
nhd_ptn_table_data(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return dbgpattble_GetData(nhd_ptn_table);
}

int
nhd_ptn_table_tiles_width(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    (void)(nhd_ptn_table);
    return DBGPATTBL_TILES_W;
}
int
nhd_ptn_table_tiles_height(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    (void)(nhd_ptn_table);
    return DBGPATTBL_TILES_H;
}
int
nhd_ptn_table_tile_width(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    (void)(nhd_ptn_table);
    return DBGPATTBL_TILE_W;
}
int
nhd_ptn_table_tile_height(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    (void)(nhd_ptn_table);
    return DBGPATTBL_TILE_H;
}

NHDPatternTable
nhd_get_ptn_table(NHConsole console, int right)
{
    NH_DECL_CONSOLE(console);
    return (NHDPatternTable)console_DbgGetPtnTbl(nh_console, right);
}

void
nhd_set_ptn_table_palette(NHConsole console, NHDPaletteSet palette)
{
    NH_DECL_CONSOLE(console);
    console_DbgSetPtnTblPal(nh_console, palette);
}

NHDSprite
nhd_oam_sprite(NHDOAM oam, int index)
{
    NHD_DECL_OAM(oam);
    return (NHDSprite)dbgoam_GetSpr(nhd_oam, index);
}

int
nhd_sprite_width(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return dbgspr_GetWidth(nhd_sprite);
}

int
nhd_sprite_height(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return dbgspr_GetHeight(nhd_sprite);
}

const NHByte *
nhd_sprite_data(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return dbgspr_GetData(nhd_sprite);
}

NHByte
nhd_sprite_y(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->Y;
}
NHByte
nhd_sprite_tile(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->Tile;
}
NHByte
nhd_sprite_attr(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->Attr;
}
NHByte
nhd_sprite_x(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->X;
}

int
nhd_sprite_palette_set(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return dbgspr_PalSet(nhd_sprite);
}
int
nhd_sprite_background(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return dbgspr_Bg(nhd_sprite);
}
int
nhd_sprite_flip_x(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return dbgspr_FlipX(nhd_sprite);
}
int
nhd_sprite_flip_y(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return dbgspr_FlipY(nhd_sprite);
}

NHDOAM
nhd_get_oam(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHDOAM)console_DbgGetOam(nh_console);
}

NHDColor
nhd_palette_color(NHDPalette palette, int index)
{
    NHD_DECL_PALETTE(palette);
    return dbgpal_GetColor(nhd_palette, index);
}

NHDPalette
nhd_get_palette(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHDPalette)console_DbgGetPal(nh_console);
}

NHCPU
nh_test_get_cpu(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHCPU)console_TestGetCpu(nh_console);
}

void
nh_test_cpu_set_entry(NHCPU cpu, NHAddr entry)
{
    NH_DECL_CPU(cpu);
    cpu_TestSetEntry(nh_cpu, entry);
}

void
nh_test_cpu_set_p(NHCPU cpu, NHByte val)
{
    NH_DECL_CPU(cpu);
    cpu_TestSetP(nh_cpu, val);
}

NHAddr
nh_test_cpu_pc(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return cpu_TestGetPc(nh_cpu);
}

NHByte
nh_test_cpu_a(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return cpu_TestGetA(nh_cpu);
}

NHByte
nh_test_cpu_x(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return cpu_TestGetX(nh_cpu);
}

NHByte
nh_test_cpu_y(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return cpu_TestGetY(nh_cpu);
}

NHByte
nh_test_cpu_p(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return cpu_TestGetP(nh_cpu);
}

NHByte
nh_test_cpu_s(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return cpu_TestGetS(nh_cpu);
}

NHCycle
nh_test_cpu_cycle(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return cpu_TestGetCycle(nh_cpu);
}

void
nh_test_cpu_instr_bytes(NHCPU cpu, NHAddr addr, NHByte bytes[3], int *size)
{
    NH_DECL_CPU(cpu);
    cpu_TestGetInstrBytes(nh_cpu, addr, bytes, size);
}
