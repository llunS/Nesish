#pragma once

#include "nesish/nesish.h"
#include "types.h"

typedef struct mementry_s mementry_s;
typedef NHErr (*memdecode_f)(const mementry_s *entry, addr_t addr, u8 **ptr);
typedef NHErr (*memgetb_f)(const mementry_s *entry, addr_t addr, u8 *val);
typedef NHErr (*memsetb_f)(const mementry_s *entry, addr_t addr, u8 val);

typedef struct mementry_s {
    addr_t Begin;
    addr_t End; // inclusive

    void *Opaque;

    bool readonly_;

    memdecode_f decode_;

    memgetb_f getbyte_;
    memsetb_f setbyte_;
} mementry_s;

void
mementry_Init(mementry_s *self, addr_t begin, addr_t end, bool readonly,
              memdecode_f decode, void *opaque);
void
mementry_InitExt(mementry_s *self, addr_t begin, addr_t end, bool readonly,
                 memgetb_f getbyte, memsetb_f setbyte, void *opaque);
