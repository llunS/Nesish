#include "cartloader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cartridge/ines.h"
#include "types.h"
#include "compilerintrinsics.h"
#include "log.h"
#include "nhassert.h"

static NHErr
loadines(const char *rompath, cart_s *cart, NHLogger *logger);

static NHErr
openRom(const char *rompath, FILE **file, NHLogger *logger);
static void
closeRom(FILE *file);

static NHErr
loadinesHeader(FILE *file, ines_s *ines, NHLogger *logger);
static NHErr
loadinesTrainer(FILE *file, ines_s *ines, NHLogger *logger);
static NHErr
loadinesPrgRom(FILE *file, ines_s *ines, NHLogger *logger);
static NHErr
loadinesChrRom(FILE *file, ines_s *ines, NHLogger *logger);

NHErr
cartld_LoadCart(const char *rompath, cartkind_e kind, cart_s *cart,
                NHLogger *logger)
{
    switch (kind) {
    case CK_INES:
        return loadines(rompath, cart, logger);
        break;

    default:
        return NH_ERR_UNIMPLEMENTED;
        break;
    }
}

NHErr
loadines(const char *rompath, cart_s *cart, NHLogger *logger)
{
    NHErr err = NH_ERR_OK;

    FILE *file = NULL;
    ines_s *ines = NULL;

    err = openRom(rompath, &file, logger);
    if (NH_FAILED(err)) {
        goto outend;
    }

    ines = malloc(sizeof(ines_s));
    if (!ines) {
        err = NH_ERR_OOM;
        goto outend;
    }
    ines_init_(ines, logger);

    err = loadinesHeader(file, ines, logger);
    if (NH_FAILED(err)) {
        goto outend;
    }
    err = loadinesTrainer(file, ines, logger);
    if (NH_FAILED(err)) {
        goto outend;
    }
    err = loadinesPrgRom(file, ines, logger);
    if (NH_FAILED(err)) {
        goto outend;
    }
    err = loadinesChrRom(file, ines, logger);
    if (NH_FAILED(err)) {
        goto outend;
    }
    err = ines_resolve_(ines);
    if (NH_FAILED(err)) {
        goto outend;
    }

outend:
    if (file) {
        closeRom(file);
        // file = NULL;
    }
    if (NH_FAILED(err)) {
        if (ines) {
            ines_Deinit(ines);
            free(ines);
            ines = NULL;
        }
    } else {
        if (ines) {
            cart->Deinit = ines_Deinit;
            cart->Validate = ines_Validate;
            cart->Powerup = ines_Powerup;
            cart->Reset = ines_Reset;
            cart->MapMemory = ines_MapMemory;
            cart->UnmapMemory = ines_UnmapMemory;
            cart->Impl = ines;
        } else {
            NH_ASSERT(false);
            err = NH_ERR_PROGRAMMING;
        }
    }

    return err;
}

NHErr
openRom(const char *rompath, FILE **file, NHLogger *logger)
{
    NH_VC_WARNING_PUSH
    NH_VC_WARNING_DISABLE(4996)
    FILE *fp = fopen(rompath, "rb");
    NH_VC_WARNING_POP
    if (!fp) {
        LOG_ERROR(logger, "File opening failed!");
        return NH_ERR_UNAVAILABLE;
    }
    *file = fp;
    return NH_ERR_OK;
}

void
closeRom(FILE *file)
{
    fclose(file);
}

NHErr
loadinesHeader(FILE *file, ines_s *ines, NHLogger *logger)
{
#define INES_HEADER_SIZE 16
    u8 buf[INES_HEADER_SIZE];
    size_t got = fread(buf, 1, sizeof buf, file);
    if (got < sizeof buf) {
        LOG_ERROR(logger, "iNES header incomplete, got %zu bytes", got);
        return NH_ERR_CORRUPTED;
    }

    inesheader_s *header = &ines->header_;
    memcpy(header->Magic, buf, 4);
    header->PrgRomSize = buf[4];
    header->ChrRomSize = buf[5];

    header->Mirror = (buf[6] >> 0) & 0x1;
    header->PersistentMem = (buf[6] >> 1) & 0x1;
    header->Trainer = (buf[6] >> 2) & 0x1;
    header->FourScrVram = (buf[6] >> 3) & 0x1;
    header->MapperLow = (buf[6] >> 4) & 0xf;

    header->VsUnisystem = (buf[7] >> 0) & 0x1;
    header->Playchoice10 = (buf[7] >> 1) & 0x1;
    header->Ines2 = (buf[7] >> 2) & 0x3;
    header->MapperHigh = (buf[7] >> 4) & 0xf;

    header->PrgRamSize = buf[8];

    return NH_ERR_OK;
}

NHErr
loadinesTrainer(FILE *file, ines_s *ines, NHLogger *logger)
{
    if (ines->header_.Trainer) {
        if (fseek(file, 512, SEEK_CUR)) {
            LOG_ERROR(logger, "Failed to skip over iNES trainer");
            return NH_ERR_CORRUPTED;
        }
    }
    return NH_ERR_OK;
}

NHErr
loadinesPrgRom(FILE *file, ines_s *ines, NHLogger *logger)
{
    if (!ines->header_.PrgRomSize) {
        LOG_ERROR(logger, "iNES PRG ROM header field is zero");
        return NH_ERR_CORRUPTED;
    }

    NHErr err = NH_ERR_OK;

    const int KiB = 1024;
    usize romBytes = ines->header_.PrgRomSize * 16 * KiB;
    ines->prgrom_ = calloc(romBytes, sizeof(u8));
    if (!ines->prgrom_) {
        err = NH_ERR_OOM;
        goto outend;
    }
    ines->prgromsize_ = romBytes;

    const usize EACH_READ = KiB;
    for (usize byteIdx = 0; byteIdx < romBytes; byteIdx += EACH_READ) {
        size_t got = fread(ines->prgrom_ + byteIdx, 1, EACH_READ, file);
        if (got < EACH_READ) {
            LOG_ERROR(logger, "iNES PRG ROM incomplete, got %zu bytes",
                      byteIdx + got);
            err = NH_ERR_CORRUPTED;
            goto outend;
        }
    }

outend:
    if (NH_FAILED(err)) {
        if (ines->prgrom_) {
            free(ines->prgrom_);
            ines->prgrom_ = NULL;
        }
    }

    return err;
}

NHErr
loadinesChrRom(FILE *file, ines_s *ines, NHLogger *logger)
{
    // use RAM instead.
    if (!ines->header_.ChrRomSize) {
        ines->usechrram_ = true;
        return NH_ERR_OK;
    }

    NHErr err = NH_ERR_OK;

    const int KiB = 1024;
    usize romBytes = ines->header_.ChrRomSize * 8 * KiB;
    ines->chrrom_ = calloc(romBytes, sizeof(u8));
    if (!ines->chrrom_) {
        err = NH_ERR_OOM;
        goto outend;
    }
    ines->chrromsize_ = romBytes;

    const usize EACH_READ = KiB;
    for (usize byteIdx = 0; byteIdx < romBytes; byteIdx += EACH_READ) {
        size_t got = fread(ines->chrrom_ + byteIdx, 1, EACH_READ, file);
        if (got < EACH_READ) {
            LOG_ERROR(logger, "iNES CHR ROM incomplete, got %zu bytes",
                      byteIdx + got);
            err = NH_ERR_CORRUPTED;
            goto outend;
        }
    }

outend:
    if (NH_FAILED(err)) {
        if (ines->chrrom_) {
            free(ines->chrrom_);
            ines->chrrom_ = NULL;
        }
    }

    return err;
}
