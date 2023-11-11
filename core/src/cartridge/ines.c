#include "ines.h"

#include "log.h"
#include "cartridge/inesromaccessor.h"
#include "cartridge/mapper/nrom.h"
#include "cartridge/mapper/mmc1.h"
#include "cartridge/mapper/cnrom.h"

#include <stdlib.h>

static mapper_s
getMapper(u8 mappernum, const inesromaccessor_s *accessor);

void
ines_init_(ines_s *self, NHLogger *logger)
{
    self->mapper_.Impl = NULL;

    self->prgrom_ = NULL;
    self->prgromsize_ = 0;
    self->chrrom_ = NULL;
    self->chrromsize_ = 0;
    self->usechrram_ = false;

    inesromaccessor_init_(&self->romaccessor_, self);

    self->logger_ = logger;
}

void
ines_Deinit(void *me)
{
    ines_s *self = (ines_s *)me;

    if (self->prgrom_)
    {
        free(self->prgrom_);
        self->prgrom_ = NULL;
    }
    if (self->chrrom_)
    {
        free(self->chrrom_);
        self->chrrom_ = NULL;
    }

    if (self->mapper_.Impl)
    {
        self->mapper_.Deinit(self->mapper_.Impl);
        free(self->mapper_.Impl);
        self->mapper_.Impl = NULL;
    }
}

NHErr
ines_resolve_(ines_s *self)
{
    // mapper
    self->mappernum_ =
        (u8)((self->header_.MapperHigh << 4) + self->header_.MapperLow);

    self->mapper_ = getMapper(self->mappernum_, &self->romaccessor_);
    if (!self->mapper_.Impl)
    {
        LOG_ERROR(self->logger_, "iNES unsupported mapper " U8FMT,
                  self->mappernum_);
        return NH_ERR_UNIMPLEMENTED;
    }

    return NH_ERR_OK;
}

NHErr
ines_Validate(const void *me)
{
    const ines_s *self = (const ines_s *)me;

    // 1. header metadata check
    // magic number: NES^Z
    if (!(self->header_.Magic[0] == 0x4e && self->header_.Magic[1] == 0x45 &&
          self->header_.Magic[2] == 0x53 && self->header_.Magic[3] == 0x1a))
    {
        LOG_ERROR(self->logger_, "iNES invalid magic number");
        return NH_ERR_CORRUPTED;
    }
    if (!self->prgrom_)
    {
        LOG_ERROR(self->logger_, "iNES empty PRG ROM");
        return NH_ERR_CORRUPTED;
    }
    if (self->header_.Ines2 != 0)
    {
        LOG_ERROR(self->logger_, "Support only iNES format for now, " UBITFMTX,
                  self->header_.Ines2);
        return NH_ERR_CORRUPTED;
    }

    // 2. runtime state check
    if (!self->mapper_.Impl)
    {
        LOG_ERROR(self->logger_, "iNES mapper uninitialized");
        return NH_ERR_UNINITIALIZED;
    }

    LOG_INFO(self->logger_, "iNES mapper " U8FMT, self->mappernum_);

    NHErr err = self->mapper_.Validate(self->mapper_.Impl);
    if (NH_FAILED(err))
    {
        return err;
    }

    return NH_ERR_OK;
}

void
ines_Powerup(void *me)
{
    ines_s *self = (ines_s *)me;

    self->mapper_.Powerup(self->mapper_.Impl);
}

void
ines_Reset(void *me)
{
    ines_s *self = (ines_s *)me;

    self->mapper_.Reset(self->mapper_.Impl);
}

void
ines_MapMemory(void *me, mmem_s *mmem, vmem_s *vmem)
{
    ines_s *self = (ines_s *)me;

    self->mapper_.MapMemory(self->mapper_.Impl, mmem, vmem);
}

void
ines_UnmapMemory(void *me, mmem_s *mmem, vmem_s *vmem)
{
    ines_s *self = (ines_s *)me;

    self->mapper_.UnmapMemory(self->mapper_.Impl, mmem, vmem);
}

#define ASSEMBLE_IMPL(interface, mappername, ...)                              \
    {                                                                          \
        mappername##_s *impl = malloc(sizeof(mappername##_s));                 \
        if (impl)                                                              \
        {                                                                      \
            mappername##_Init(impl, __VA_ARGS__);                              \
            interface.Deinit = mappername##_Deinit;                            \
            interface.Validate = mappername##_Validate;                        \
            interface.Powerup = mappername##_Powerup;                          \
            interface.Reset = mappername##_Reset;                              \
            interface.MapMemory = mappername##_MapMemory;                      \
            interface.UnmapMemory = mappername##_UnmapMemory;                  \
            interface.Impl = impl;                                             \
        }                                                                      \
    }

mapper_s
getMapper(u8 mappernum, const inesromaccessor_s *accessor)
{
    mapper_s interface = {.Impl = NULL};
    switch (mappernum)
    {
        case 0:
            ASSEMBLE_IMPL(interface, nrom, accessor);
            break;

        case 1:
            ASSEMBLE_IMPL(interface, mmc1, accessor, MMC1B);
            break;

        case 3:
            ASSEMBLE_IMPL(interface, cnrom, accessor);
            break;

        default:
            break;
    }
    return interface;
}
