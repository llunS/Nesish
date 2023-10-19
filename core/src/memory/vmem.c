#include "vmem.h"

#include <string.h>

static NHErr
ntDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    vmem_s *self = (vmem_s *)entry->Opaque;

    return membase_decodeAddr(&self->Base, addr & NH_NT_MIRROR_ADDR_MASK, ptr);
}

static NHErr
palDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    vmem_s *self = (vmem_s *)entry->Opaque;

    addr_t addrval = (addr & NH_PALETTE_ADDR_MASK) | entry->Begin;
    // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of
    // $3F00/$3F04/$3F08/$3F0C.
    if ((addrval & NH_PALETTE_ADDR_MIRROR_MASK) == 0)
    {
        addrval &= NH_PALETTE_ADDR_BG_MASK;
    }

    addr_t index = addrval - entry->Begin;
    *ptr = &self->pal_[index];
    return NH_ERR_OK;
}

static NHErr
restDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    vmem_s *self = (vmem_s *)entry->Opaque;

    // Valid addresses are $0000-$3FFF; higher addresses will be
    // mirrored down.
    return membase_decodeAddr(&self->Base, addr & NH_PPU_INVALID_ADDR_MASK,
                              ptr);
}

bool
vmem_Init(vmem_s *self, NHLogger *logger)
{
    if (!membase_Init(&self->Base, NH_ADDRESSABLE_SIZE, VMP_SIZE, VMP_NAH,
                      logger))
    {
        return false;
    }

    memset(self->ram_, 0, sizeof(self->ram_));
    memset(self->pal_, 0, sizeof(self->pal_));

    // Nametable mirror
    {
        mementry_s entry;
        mementry_Init(&entry, NH_NT_MIRROR_ADDR_HEAD, NH_NT_MIRROR_ADDR_TAIL,
                      false, ntDecode, self);
        membase_SetMapping(&self->Base, VMP_NT_MIR, entry);
    }
    // Palette
    {
        mementry_s entry;
        mementry_Init(&entry, NH_PALETTE_ADDR_HEAD, NH_PALETTE_ADDR_TAIL, false,
                      palDecode, self);
        membase_SetMapping(&self->Base, VMP_PAL, entry);
    }
    // Invalid addresses
    {
        mementry_s entry;
        mementry_Init(&entry, NH_PPU_INVALID_ADDR_HEAD,
                      NH_PPU_INVALID_ADDR_TAIL, false, restDecode, self);
        membase_SetMapping(&self->Base, VMP_REST, entry);
    }

    return true;
}

void
vmem_Deinit(vmem_s *self)
{
    membase_Deinit(&self->Base);
}

static mirmode_e
fixedMirrorCb(void *opaque)
{
    vmem_s *self = (vmem_s *)opaque;
    return self->mirFixedMode_;
}

void
vmem_SetMirrorFixed(vmem_s *self, mirmode_e mode)
{
    self->mirFixedMode_ = mode;
    vmem_SetMirrorDyn(self, fixedMirrorCb, self);
}

static NHErr
mirrorDynDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    vmem_s *self = (vmem_s *)entry->Opaque;

    // require at least C11, not that much needed
    // static_assert(sizeof(self->ram_) == sizeof(u8) * 2 * NH_NT_ONE_SIZE,
    //   "Wrong internal ram size");

    mirmode_e mode = self->mirDynDecode_(self->mirOpaque_);
    switch (mode)
    {
        case MM_H:
        {
            // 0x2400 -> 0x2000
            // 0x2C00 -> 0x2800
            addr_t addrval = addr & NH_NT_H_MIRROR_ADDR_MASK;
            addr_t index = addrval - entry->Begin -
                           (addrval >= NH_NT_2_ADDR_HEAD ? NH_NT_ONE_SIZE : 0);
            *ptr = &self->ram_[index];
            return NH_ERR_OK;
        }
        break;

        case MM_V:
        {
            // 0x2800 -> 0x2000
            // 0x2C00 -> 0x2400
            addr_t addrval = addr & NH_NT_V_MIRROR_ADDR_MASK;
            addr_t index = addrval - entry->Begin;
            *ptr = &self->ram_[index];
            return NH_ERR_OK;
        }
        break;

        case MM_1LOW:
        {
            // 0x2400 -> 0x2000
            // 0x2800 -> 0x2000
            // 0x2C00 -> 0x2000
            addr_t addrval = addr & NH_NT_1_MIRROR_ADDR_MASK;
            addr_t index = addrval - entry->Begin;
            *ptr = &self->ram_[index];
            return NH_ERR_OK;
        }
        break;

        case MM_1HIGH:
        {
            // 0x2400 -> 0x2000
            // 0x2800 -> 0x2000
            // 0x2C00 -> 0x2000
            addr_t addrval = addr & NH_NT_1_MIRROR_ADDR_MASK;
            addr_t index = addrval - entry->Begin + NH_NT_ONE_SIZE;
            *ptr = &self->ram_[index];
            return NH_ERR_OK;
        }
        break;

        default:
        {
            // Impossible
            *ptr = &self->ram_[0];
            return NH_ERR_OK;
        }
        break;
    }
}

void
vmem_SetMirrorDyn(vmem_s *self, mirmode_f modecb, void *opaque)
{
    self->mirDynDecode_ = modecb;
    self->mirOpaque_ = opaque;
    mementry_s entry;
    mementry_Init(&entry, NH_NT_ADDR_HEAD, NH_NT_ADDR_TAIL, false,
                  mirrorDynDecode, self);
    membase_SetMapping(&self->Base, VMP_NT, entry);
}

void
vmem_UnsetMirror(vmem_s *self)
{
    membase_UnsetMapping(&self->Base, VMP_NT);
}

NHErr
vmem_GetB(const vmem_s *self, addr_t addr, u8 *val)
{
    // https://www.nesdev.org/wiki/Open_bus_behavior#PPU_open_bus
    NHErr err = membase_GetB(&self->Base, addr, val);
    if (err == NH_ERR_UNAVAILABLE)
    {
        *val = addr & 0x00FF;
        err = NH_ERR_OK;
    }
    return err;
}

u8
vmem_GetPalByte(vmem_s *self, int idx)
{
    return self->pal_[idx];
}
