#include "membase.h"

#include <stdlib.h>

#include "log.h"

typedef struct entrykv_s {
    mp_e K;
    const mementry_s *V;
} entrykv_s;

static entrykv_s
getEntryKv(const membase_s *self, addr_t addr);
static NHErr
decodeAddrImpl(const membase_s *self, const entrykv_s *kv, addr_t addr,
               u8 **ptr);

typedef struct entryele_s {
    mementry_s Entry;
    bool Valid;
} entryele_s;

bool
membase_Init(membase_s *self, usize memsize, mp_e ptcount, mp_e invalidpt,
             NHLogger *logger)
{
    self->mapreg_ = NULL;
    self->mapentries_ = NULL;
    self->logger = logger;

    if (0 == memsize || ADDRMAX < memsize - 1)
    {
        LOG_FATAL(self->logger, "memory size is 0 or too large " USIZEFMT,
                  memsize);
        goto outerr;
    }

    // allocate and init self->mapreg_
    self->mapreg_ = malloc(memsize * sizeof(mp_e));
    if (!self->mapreg_)
    {
        goto outerr;
    }
    for (usize i = 0; i < memsize; ++i)
    {
        self->mapreg_[i] = invalidpt;
    }
    self->memsize_ = memsize;
    self->invalidpt_ = invalidpt;

    // allocate and init self->mapentries_
    // each entry's valid is false due to calloc
    self->mapentries_ = calloc(ptcount, sizeof(entryele_s));
    if (!self->mapentries_)
    {
        goto outerr;
    }

    return true;
outerr:
    membase_Deinit(self);
    return false;
}

void
membase_Deinit(membase_s *self)
{
    if (self->mapentries_)
    {
        free(self->mapentries_);
        self->mapentries_ = NULL;
    }
    if (self->mapreg_)
    {
        free(self->mapreg_);
        self->mapreg_ = NULL;
    }
}

NHErr
membase_GetB(const membase_s *self, addr_t addr, u8 *val)
{
    entrykv_s kv = getEntryKv(self, addr);
    const mementry_s *entry = kv.V;
    if (!entry)
    {
        // No device active with this address
        return NH_ERR_UNAVAILABLE;
    }

    if (entry->getbyte_)
    {
        NHErr err = entry->getbyte_(entry, addr, val);
        return err;
    }
    else
    {
        u8 *bptr = NULL;
        NHErr err = decodeAddrImpl(self, &kv, addr, &bptr);
        if (NH_FAILED(err))
        {
            return err;
        }
        if (!bptr)
        {
            return NH_ERR_PROGRAMMING;
        }

        *val = *bptr;
        return NH_ERR_OK;
    }
}

NHErr
membase_SetB(membase_s *self, addr_t addr, u8 val)
{
    entrykv_s kv = getEntryKv(self, addr);
    const mementry_s *entry = kv.V;
    if (!entry)
    {
        // No device active with this address
        return NH_ERR_UNAVAILABLE;
    }

    if (entry->readonly_)
    {
        // Simply ignore the write to read-only memory
        return NH_ERR_READ_ONLY;
    }

    if (entry->setbyte_)
    {
        NHErr err = entry->setbyte_(entry, addr, val);
        return err;
    }
    else
    {
        u8 *bptr = NULL;
        NHErr err = decodeAddrImpl(self, &kv, addr, &bptr);
        if (NH_FAILED(err))
        {
            return err;
        }
        if (!bptr)
        {
            return NH_ERR_PROGRAMMING;
        }
        *bptr = val;
        return NH_ERR_OK;
    }
}

/// @note User must ensure address ranges of different mapping points don't
/// overlap.
void
membase_SetMapping(membase_s *self, mp_e pt, mementry_s entry)
{
    // Ensure valid entry value.
    if (entry.Begin > entry.End)
    {
        LOG_ERROR(self->logger,
                  "Invalid mapping entry range: " ADDRFMT ", " ADDRFMT,
                  entry.Begin, entry.End);
        return;
    }
    if (entry.Begin >= self->memsize_ || entry.End >= self->memsize_)
    {
        LOG_ERROR(self->logger,
                  "Invalid mapping entry range: [" ADDRFMT ", " ADDRFMT
                  "] in " USIZEFMT,
                  entry.Begin, entry.End, self->memsize_);
        return;
    }
    if (entry.decode_ == NULL &&
        (entry.getbyte_ == NULL || entry.setbyte_ == NULL))
    {
        LOG_ERROR(self->logger, "Empty mapping callbacks");
        return;
    }

    // unset, if any.
    membase_UnsetMapping(self, pt);

    mp_e eidx = pt;
    self->mapentries_[eidx].Entry = entry;
    self->mapentries_[eidx].Valid = true;
    for (usize i = entry.Begin; i <= entry.End; ++i)
    {
        self->mapreg_[i] = pt;
    }
}

void
membase_UnsetMapping(membase_s *self, mp_e pt)
{
    mp_e eidx = pt;
    if (!self->mapentries_[eidx].Valid)
    {
        return;
    }

    const mementry_s *entry = &self->mapentries_[eidx].Entry;
    for (usize i = entry->Begin; i <= entry->End; ++i)
    {
        self->mapreg_[i] = self->invalidpt_;
    }

    self->mapentries_[eidx].Valid = false;
}

entrykv_s
getEntryKv(const membase_s *self, addr_t addr)
{
    mp_e mp = self->mapreg_[addr];
    if (mp == self->invalidpt_)
    {
        // Invalid address points to nothing.
        return (entrykv_s){self->invalidpt_, NULL};
    }

    mp_e eidx = mp;
    if (!self->mapentries_[eidx].Valid)
    {
        return (entrykv_s){self->invalidpt_, NULL};
    }

    return (entrykv_s){mp, &self->mapentries_[eidx].Entry};
}

NHErr
membase_decodeAddr(const membase_s *self, addr_t addr, u8 **ptr)
{
    entrykv_s kv = getEntryKv(self, addr);
    return decodeAddrImpl(self, &kv, addr, ptr);
}

NHErr
decodeAddrImpl(const membase_s *self, const entrykv_s *kv, addr_t addr,
               u8 **ptr)
{
    const mp_e mp = kv->K;
    const mementry_s *entry = kv->V;

    if (!entry)
    {
        return NH_ERR_UNAVAILABLE;
    }

    u8 *bptr = NULL;
    NHErr err = entry->decode_(entry, addr, &bptr);
    if (NH_FAILED(err))
    {
        return err;
    }

    if (!bptr)
    {
        LOG_ERROR(self->logger,
                  "Registered mapping can not handle address "
                  "decoding: " ADDRFMT ", " MPFMT ", " ADDRFMT ", " ADDRFMT,
                  addr, mp, entry->Begin, entry->End);
    }
    *ptr = bptr;
    return NH_ERR_OK;
}
