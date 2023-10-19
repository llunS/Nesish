#include "mmem.h"

#include <string.h>

static NHErr
internalRamDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    u8 *ram = (u8 *)entry->Opaque;
    addr_t addrval = addr & NH_RAM_ADDR_MASK;
    *ptr = ram + addrval;
    return NH_ERR_OK;
}

bool
mmem_Init(mmem_s *self, NHLogger *logger)
{
    if (!membase_Init(&self->Base, NH_ADDRESSABLE_SIZE, MMP_SIZE, MMP_NAH,
                      logger))
    {
        return false;
    }

    memset(self->ram_, 0, sizeof(self->ram_));
    self->readlatch_ = 0;

    // Internal RAM space mapping
    {
        mementry_s entry;
        mementry_Init(&entry, NH_RAM_ADDR_HEAD, NH_RAM_ADDR_TAIL, false,
                      internalRamDecode, self->ram_);
        membase_SetMapping(&self->Base, MMP_RAM, entry);
    }

    return true;
}

void
mmem_Deinit(mmem_s *self)
{
    membase_Deinit(&self->Base);
}

NHErr
mmem_GetB(const mmem_s *self, addr_t addr, u8 *val)
{
    // https://www.nesdev.org/wiki/Open_bus_behavior
    NHErr err = membase_GetB(&self->Base, addr, val);
    if (!NH_FAILED(err))
    {
        ((mmem_s *)self)->readlatch_ = *val;
    }
    else
    {
        if (err == NH_ERR_UNAVAILABLE || err == NH_ERR_WRITE_ONLY)
        {
            *val = self->readlatch_;
            err = NH_ERR_OK;
        }
    }
    return err;
}

u8
mmem_GetLatch(const mmem_s *self)
{
    return self->readlatch_;
}

void
mmem_OverrideLatch(mmem_s *self, u8 val)
{
    self->readlatch_ = val;
}

NHErr
mmem_SetBulk(mmem_s *self, addr_t begin, addr_t end, u8 val)
{
    if (begin >= end)
    {
        return NH_ERR_INVALID_ARGUMENT;
    }

    // optimize only simple cases for now.
    if (begin < NH_INTERNAL_RAM_SIZE && end < NH_INTERNAL_RAM_SIZE)
    {
        memset(self->ram_ + begin, val, end - begin);
        return NH_ERR_OK;
    }

    // For now, only support consecutive and non-page-crossing range.
    return NH_ERR_INVALID_ARGUMENT;
}
