#pragma once

#include "nesish/nesish.h"
#include "types.h"
#include "memory/mementry.h"

// each enumerator must be unique and valid array index
typedef int mp_e;
#define MPFMT "%d"

typedef struct entryele_s entryele_s;

typedef struct membase_s {
    mp_e *mapreg_;
    usize memsize_;
    mp_e invalidpt_;

    entryele_s *mapentries_;

    NHLogger *logger;
} membase_s;

bool
membase_Init(membase_s *self, usize memsize, mp_e ptcount, mp_e invalidpt,
             NHLogger *logger);
void
membase_Deinit(membase_s *self);

NHErr
membase_GetB(const membase_s *self, addr_t addr, u8 *val);
NHErr
membase_SetB(membase_s *self, addr_t addr, u8 val);

void
membase_SetMapping(membase_s *self, mp_e pt, mementry_s entry);
void
membase_UnsetMapping(membase_s *self, mp_e pt);

NHErr
membase_decodeAddr(const membase_s *self, addr_t addr, u8 **ptr);
