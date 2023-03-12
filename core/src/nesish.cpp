#include "nesish/nesish.h"

#include "console.hpp"

#define NH_DECL_CONSOLE(handle)                                                \
    nh::Console *nh_console = (nh::Console *)(handle);
#define NH_DECL_CPU(handle) nh::CPU *nh_cpu = (nh::CPU *)(handle);
#define NH_DECL_FRM(handle)                                                    \
    const nh::FrameBuffer *nh_frame = (const nh::FrameBuffer *)(handle);

#define NHD_DECL_PTNTBL(handle)                                                \
    const nhd::PatternTable *nhd_ptn_table =                                   \
        (const nhd::PatternTable *)(handle);
#define NHD_DECL_OAM(handle)                                                   \
    const nhd::OAM *nhd_oam = (const nhd::OAM *)(handle);
#define NHD_DECL_SPRITE(handle)                                                \
    const nhd::Sprite *nhd_sprite = (const nhd::Sprite *)(handle);
#define NHD_DECL_PALETTE(handle)                                               \
    const nhd::Palette *nhd_palette = (const nhd::Palette *)(handle);

NHConsole
nh_new_console(NHLogger *logger)
{
    return (NHConsole) new nh::Console(logger);
}

void
nh_release_console(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    delete nh_console;
}

void
nh_plug_ctrl(NHConsole console, NHCtrlSlot slot, NHController *ctrl)
{
    NH_DECL_CONSOLE(console);
    nh_console->plug_controller(slot, ctrl);
}

void
nh_unplug_ctrl(NHConsole console, NHCtrlSlot slot)
{
    NH_DECL_CONSOLE(console);
    nh_console->unplug_controller(slot);
}

NHErr
nh_insert_cart(NHConsole console, const char *rom_path)
{
    NH_DECL_CONSOLE(console);
    std::string s{rom_path};
    return nh_console->insert_cartridge(s);
}

void
nh_power_up(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    nh_console->power_up();
}

void
nh_reset(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    nh_console->reset();
}

NHCycle
nh_advance(NHConsole console, double delta)
{
    NH_DECL_CONSOLE(console);
    return nh_console->advance(delta);
}

int
nh_tick(NHConsole console, int *cpu_instr)
{
    NH_DECL_CONSOLE(console);
    bool b1;
    bool b0 = nh_console->tick(&b1);
    if (cpu_instr)
    {
        *cpu_instr = b1;
    }
    return b0;
}

double
nh_elapsed(NHConsole console, NHCycle ticks)
{
    NH_DECL_CONSOLE(console);
    return nh_console->elapsed(ticks);
}

int
nh_frm_width(NHFrame frame)
{
    NH_DECL_FRM(frame);
    return nh_frame->WIDTH;
}
int
nh_frm_height(NHFrame frame)
{
    NH_DECL_FRM(frame);
    return nh_frame->HEIGHT;
}
const NHByte *
nh_frm_data(NHFrame frame)
{
    NH_DECL_FRM(frame);
    return nh_frame->get_data();
}

NHFrame
nh_get_frm(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHFrame)&nh_console->get_frame();
}

float
nh_get_sample_rate(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return nh_console->get_sample_rate();
}
float
nh_get_sample(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return nh_console->get_sample();
}

void
nhd_turn_debug_on(NHConsole console, NHDFlags flag)
{
    NH_DECL_CONSOLE(console);
    nh_console->set_debug_on(flag);
}
void
nhd_turn_debug_off(NHConsole console, NHDFlags flag)
{
    NH_DECL_CONSOLE(console);
    nh_console->set_debug_off(flag);
}

int
nhd_ptn_table_width(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return nhd_ptn_table->get_width();
}
int
nhd_ptn_table_height(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return nhd_ptn_table->get_height();
}
const NHByte *
nhd_ptn_table_data(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return nhd_ptn_table->get_data();
}

int
nhd_ptn_table_tiles_width(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return nhd_ptn_table->get_tiles_width();
}
int
nhd_ptn_table_tiles_height(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return nhd_ptn_table->get_tiles_height();
}
int
nhd_ptn_table_tile_width(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return nhd_ptn_table->get_tile_width();
}
int
nhd_ptn_table_tile_height(NHDPatternTable ptn_table)
{
    NHD_DECL_PTNTBL(ptn_table);
    return nhd_ptn_table->get_tile_height();
}

NHDPatternTable
nhd_get_ptn_table(NHConsole console, int right)
{
    NH_DECL_CONSOLE(console);
    return (NHDPatternTable)&nh_console->dbg_get_ptn_tbl(right);
}

void
nhd_set_ptn_table_palette(NHConsole console, NHDPaletteSet palette)
{
    NH_DECL_CONSOLE(console);
    nh_console->dbg_set_ptn_tbl_palette(palette);
}

NHDSprite
nhd_oam_sprite(NHDOAM oam, int index)
{
    NHD_DECL_OAM(oam);
    return (NHDSprite)&nhd_oam->get_sprite(index);
}

int
nhd_sprite_width(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->get_width();
}

int
nhd_sprite_height(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->get_height();
}

const NHByte *
nhd_sprite_data(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->get_data();
}

NHByte
nhd_sprite_y(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->y;
}
NHByte
nhd_sprite_tile(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->tile;
}
NHByte
nhd_sprite_attr(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->attr;
}
NHByte
nhd_sprite_x(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->x;
}

int
nhd_sprite_palette_set(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->palette_set();
}
int
nhd_sprite_background(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->background();
}
int
nhd_sprite_flip_x(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->flip_x();
}
int
nhd_sprite_flip_y(NHDSprite sprite)
{
    NHD_DECL_SPRITE(sprite);
    return nhd_sprite->flip_y();
}

NHDOAM
nhd_get_oam(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHDOAM)&nh_console->dbg_get_oam();
}

NHDColor
nhd_palette_color(NHDPalette palette, int index)
{
    NHD_DECL_PALETTE(palette);
    return nhd_palette->get_color(index);
}

NHDPalette
nhd_get_palette(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHDPalette)&nh_console->dbg_get_palette();
}

NHCPU
nh_test_get_cpu(NHConsole console)
{
    NH_DECL_CONSOLE(console);
    return (NHCPU)nh_console->test_get_cpu();
}

void
nh_test_cpu_set_entry(NHCPU cpu, NHAddr entry)
{
    NH_DECL_CPU(cpu);
    nh_cpu->test_set_entry(entry);
}

void
nh_test_cpu_set_p(NHCPU cpu, NHByte val)
{
    NH_DECL_CPU(cpu);
    nh_cpu->test_set_p(val);
}

NHAddr
nh_test_cpu_pc(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return nh_cpu->test_get_pc();
}

NHByte
nh_test_cpu_a(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return nh_cpu->test_get_a();
}

NHByte
nh_test_cpu_x(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return nh_cpu->test_get_x();
}

NHByte
nh_test_cpu_y(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return nh_cpu->test_get_y();
}

NHByte
nh_test_cpu_p(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return nh_cpu->test_get_p();
}

NHByte
nh_test_cpu_s(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return nh_cpu->test_get_s();
}

NHCycle
nh_test_cpu_cycle(NHCPU cpu)
{
    NH_DECL_CPU(cpu);
    return nh_cpu->test_get_cycle();
}

void
nh_test_cpu_instr_bytes(NHCPU cpu, NHAddr addr, NHByte bytes[3], int *size)
{
    NH_DECL_CPU(cpu);
    auto vec = nh_cpu->test_get_instr_bytes(addr);
    for (decltype(vec.size()) i = 0; i < vec.size(); ++i)
    {
        bytes[i] = vec[i];
    }
    if (size)
    {
        *size = (int)vec.size();
    }
}
