#include "cartridge_loader.hpp"

#include <cstdio>
#include <cstring>

#include "nesish/nesish.h"
#include "cartridge/ines.hpp"
#include "types.hpp"
#include "nhbase/filesystem.hpp"
#include "nhbase/vc_intrinsics.hpp"
#include "log.hpp"
#include "assert.hpp"

namespace nh {

static NHErr
pv_open_rom(const std::string &i_rom_path, std::FILE **o_file,
            NHLogger *i_logger);
static void
pv_close_rom(std::FILE *i_file);

NHErr
CartridgeLoader::load_cartridge(const std::string &i_rom_path,
                                CartridgeType i_type, Cartridge **o_cartridge,
                                NHLogger *i_logger)
{
    switch (i_type)
    {
        case CartridgeType::INES:
            return load_ines(i_rom_path, o_cartridge, i_logger);
            break;

        default:
            return NH_ERR_UNIMPLEMENTED;
            break;
    }
}

NHErr
CartridgeLoader::load_ines(const std::string &i_rom_path,
                           Cartridge **o_cartridge, NHLogger *i_logger)
{
    NHErr err = NH_ERR_OK;

    std::FILE *file = nullptr;
    INES *o_ines = nullptr;

    err = pv_open_rom(i_rom_path, &file, i_logger);
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }

    o_ines = new INES(i_logger);
    err = pv_load_ines_header(file, o_ines, i_logger);
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }
    err = pv_load_ines_trainer(file, o_ines, i_logger);
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }
    err = pv_load_ines_prg_rom(file, o_ines, i_logger);
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }
    err = pv_load_ines_chr_rom(file, o_ines, i_logger);
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }
    err = o_ines->resolve();
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }

l_cleanup:
    if (file)
        pv_close_rom(file);
    if (NH_FAILED(err))
    {
        if (o_ines)
            delete o_ines;
        o_ines = nullptr;
    }
    else
    {
        if (o_ines)
        {
            *o_cartridge = o_ines;
        }
        else
        {
            NH_ASSERT(false);
            err = NH_ERR_PROGRAMMING;
        }
    }

    return err;
}

NHErr
pv_open_rom(const std::string &i_rom_path, std::FILE **o_file,
            NHLogger *i_logger)
{
    if (!nb::file_exists(i_rom_path))
    {
        NH_LOG_ERROR(i_logger, "File opening failed!");
        return NH_ERR_UNAVAILABLE;
    }

    NB_VC_WARNING_PUSH
    NB_VC_WARNING_DISABLE(4996)
    FILE *fp = std::fopen(i_rom_path.c_str(), "rb");
    NB_VC_WARNING_POP
    if (!fp)
    {
        NH_LOG_ERROR(i_logger, "File opening failed!");
        return NH_ERR_UNAVAILABLE;
    }
    *o_file = fp;
    return NH_ERR_OK;
}

void
pv_close_rom(std::FILE *i_file)
{
    std::fclose(i_file);
}

NHErr
CartridgeLoader::pv_load_ines_header(std::FILE *i_file, INES *io_ines,
                                     NHLogger *i_logger)
{
#define INES_HEADER_SIZE 16
    Byte buf[INES_HEADER_SIZE];
    auto got = std::fread(buf, 1, sizeof buf, i_file);
    if (got < sizeof buf)
    {
        NH_LOG_ERROR(i_logger, "iNES header incomplete, got {} bytes", got);
        return NH_ERR_CORRUPTED;
    }

    auto *header = &io_ines->m_header;
    std::memcpy(header->nes_magic, buf, 4);
    header->prg_rom_size = buf[4];
    header->chr_rom_size = buf[5];

    header->mirror = (buf[6] >> 0) & 0x1;
    header->persistent_memory = (buf[6] >> 1) & 0x1;
    header->trainer = (buf[6] >> 2) & 0x1;
    header->four_screen_vram = (buf[6] >> 3) & 0x1;
    header->mapper_lower = (buf[6] >> 4) & 0xf;

    header->vs_unisystem = (buf[7] >> 0) & 0x1;
    header->playchoice_10 = (buf[7] >> 1) & 0x1;
    header->ines2 = (buf[7] >> 2) & 0x3;
    header->mapper_higher = (buf[7] >> 4) & 0xf;

    header->prg_ram_size = buf[8];

    return NH_ERR_OK;
}

NHErr
CartridgeLoader::pv_load_ines_trainer(std::FILE *i_file, INES *io_ines,
                                      NHLogger *i_logger)
{
    if (io_ines->m_header.trainer)
    {
        if (std::fseek(i_file, 512, SEEK_CUR))
        {
            NH_LOG_ERROR(i_logger, "Failed to skip over iNES trainer");
            return NH_ERR_CORRUPTED;
        }
    }
    return NH_ERR_OK;
}

NHErr
CartridgeLoader::pv_load_ines_prg_rom(std::FILE *i_file, INES *io_ines,
                                      NHLogger *i_logger)
{
    if (!io_ines->m_header.prg_rom_size)
    {
        NH_LOG_ERROR(i_logger, "iNES PRG ROM header field is zero");
        return NH_ERR_CORRUPTED;
    }

    NHErr err = NH_ERR_OK;

    constexpr int KiB = 1024;
    std::size_t rom_bytes = io_ines->m_header.prg_rom_size * 16 * KiB;
    io_ines->m_prg_rom = new Byte[rom_bytes]();
    io_ines->m_prg_rom_size = rom_bytes;

    constexpr std::size_t EACH_READ = KiB;
    for (std::size_t byte_idx = 0; byte_idx < rom_bytes; byte_idx += EACH_READ)
    {
        NB_VC_WARNING_PUSH
        NB_VC_WARNING_DISABLE(6386) // false positive
        auto got =
            std::fread(io_ines->m_prg_rom + byte_idx, 1, EACH_READ, i_file);
        NB_VC_WARNING_POP
        if (got < EACH_READ)
        {
            NH_LOG_ERROR(i_logger, "iNES PRG ROM incomplete, got {} bytes",
                         byte_idx + got);
            err = NH_ERR_CORRUPTED;
            goto l_cleanup;
        }
    }

l_cleanup:
    if (NH_FAILED(err))
    {
        delete[] io_ines->m_prg_rom;
        io_ines->m_prg_rom = nullptr;
    }

    return err;
}

NHErr
CartridgeLoader::pv_load_ines_chr_rom(std::FILE *i_file, INES *io_ines,
                                      NHLogger *i_logger)
{
    // use RAM instead.
    if (!io_ines->m_header.chr_rom_size)
    {
        io_ines->m_use_chr_ram = true;
        return NH_ERR_OK;
    }

    NHErr err = NH_ERR_OK;

    constexpr int KiB = 1024;
    std::size_t rom_bytes = io_ines->m_header.chr_rom_size * 8 * KiB;
    io_ines->m_chr_rom = new Byte[rom_bytes]();
    io_ines->m_chr_rom_size = rom_bytes;

    constexpr std::size_t EACH_READ = KiB;
    for (std::size_t byte_idx = 0; byte_idx < rom_bytes; byte_idx += EACH_READ)
    {
        NB_VC_WARNING_PUSH
        NB_VC_WARNING_DISABLE(6386) // false positive
        auto got =
            std::fread(io_ines->m_chr_rom + byte_idx, 1, EACH_READ, i_file);
        NB_VC_WARNING_POP
        if (got < EACH_READ)
        {
            NH_LOG_ERROR(i_logger, "iNES CHR ROM incomplete, got {} bytes",
                         byte_idx + got);
            err = NH_ERR_CORRUPTED;
            goto l_cleanup;
        }
    }

l_cleanup:
    if (NH_FAILED(err))
    {
        delete[] io_ines->m_chr_rom;
        io_ines->m_chr_rom = nullptr;
    }

    return err;
}

} // namespace nh
