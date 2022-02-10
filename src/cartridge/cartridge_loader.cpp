#include "cartridge_loader.hpp"

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include "cartridge/ines.hpp"
#include "common/logger.hpp"

namespace ln {

static Error
pvt_open_rom(const std::string &i_rom_path, std::FILE **o_file);
static void
pvt_close_rom(std::FILE *i_file);

Error
CartridgeLoader::load_cartridge(const std::string &i_rom_path,
                                CartridgeType i_type, Cartridge **o_cartridge)
{
    switch (i_type)
    {
        case CartridgeType::INES:
            return load_ines(i_rom_path, o_cartridge);
            break;

        default:
            return Error::UNIMPLEMENTED;
            break;
    }
}

Error
CartridgeLoader::load_ines(const std::string &i_rom_path,
                           Cartridge **o_cartridge)
{
    auto err = Error::OK;

    std::FILE *file = nullptr;
    INES *o_ines = nullptr;

    err = pvt_open_rom(i_rom_path, &file);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }

    o_ines = new INES();
    err = pvt_load_ines_header(file, o_ines);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }
    err = pvt_load_ines_trainer(file, o_ines);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }
    err = pvt_load_ines_prg_rom(file, o_ines);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }
    err = pvt_load_ines_chr_rom(file, o_ines);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }
    o_ines->resolve();

l_cleanup:
    if (file)
        pvt_close_rom(file);
    if (LN_FAILED(err))
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
            assert(false);
            err = Error::PROGRAMMING;
        }
    }

    return err;
}

Error
pvt_open_rom(const std::string &i_rom_path, std::FILE **o_file)
{
    FILE *fp = std::fopen(i_rom_path.c_str(), "rb");
    if (!fp)
    {
        get_logger()->error("File opening failed!");
        return Error::UNAVAILABLE;
    }
    *o_file = fp;
    return Error::OK;
}

void
pvt_close_rom(std::FILE *i_file)
{
    std::fclose(i_file);
}

Error
CartridgeLoader::pvt_load_ines_header(std::FILE *i_file, INES *io_ines)
{
#define INES_HEADER_SIZE 16
    uint8_t buf[INES_HEADER_SIZE];
    auto got = std::fread(buf, 1, sizeof buf, i_file);
    if (got < sizeof buf)
    {
        get_logger()->error("iNES header incomplete, got {} bytes.", got);
        return Error::CORRUPTED;
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
    header->mapper_upper = (buf[7] >> 4) & 0xf;

    header->prg_ram_size = buf[8];

    return Error::OK;
}

Error
CartridgeLoader::pvt_load_ines_trainer(std::FILE *i_file, INES *io_ines)
{
    if (io_ines->m_header.trainer)
    {
        if (std::fseek(i_file, 512, SEEK_CUR))
        {
            get_logger()->error("Failed to skip over iNES trainer.");
            return Error::CORRUPTED;
        }
    }
    return Error::OK;
}

Error
CartridgeLoader::pvt_load_ines_prg_rom(std::FILE *i_file, INES *io_ines)
{
    if (!io_ines->m_header.prg_rom_size)
    {
        get_logger()->error("iNES PRG ROM header field is zero.");
        return Error::CORRUPTED;
    }

    auto err = Error::OK;

    std::size_t rom_bytes = io_ines->m_header.prg_rom_size * 16 * 1024;
    io_ines->m_prg_rom = new uint8_t[rom_bytes]();

    constexpr std::size_t EACH_READ = 1024;
    for (std::size_t byte_idx = 0; byte_idx < rom_bytes; byte_idx += EACH_READ)
    {
        auto got =
            std::fread(io_ines->m_prg_rom + byte_idx, 1, EACH_READ, i_file);
        if (got < EACH_READ)
        {
            get_logger()->error("iNES PRG ROM incomplete, got {} bytes.",
                                byte_idx + got);
            err = Error::CORRUPTED;
            goto l_cleanup;
        }
    }

l_cleanup:
    if (LN_FAILED(err))
    {
        delete[] io_ines->m_prg_rom;
        io_ines->m_prg_rom = nullptr;
    }

    return err;
}

Error
CartridgeLoader::pvt_load_ines_chr_rom(std::FILE *i_file, INES *io_ines)
{
    // use RAM instead.
    if (!io_ines->m_header.chr_rom_size)
    {
        return Error::OK;
    }

    auto err = Error::OK;

    std::size_t rom_bytes = io_ines->m_header.chr_rom_size * 8 * 1024;
    io_ines->m_chr_rom = new uint8_t[rom_bytes]();

    constexpr std::size_t EACH_READ = 1024;
    for (std::size_t byte_idx = 0; byte_idx < rom_bytes; byte_idx += EACH_READ)
    {
        auto got =
            std::fread(io_ines->m_chr_rom + byte_idx, 1, EACH_READ, i_file);
        if (got < EACH_READ)
        {
            get_logger()->error("iNES CHR ROM incomplete, got {} bytes.",
                                byte_idx + got);
            err = Error::CORRUPTED;
            goto l_cleanup;
        }
    }

l_cleanup:
    if (LN_FAILED(err))
    {
        delete[] io_ines->m_chr_rom;
        io_ines->m_chr_rom = nullptr;
    }

    return err;
}

} // namespace ln
