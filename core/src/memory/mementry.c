#include "mementry.h"

static void
init(mementry_s *self, addr_t begin, addr_t end, bool readonly,
     memdecode_f decode, memgetb_f getbyte, memsetb_f setbyte, void *opaque)
{
    self->Begin = begin;
    self->End = end;
    self->Opaque = opaque;
    self->readonly_ = readonly;
    self->decode_ = decode;
    self->getbyte_ = getbyte;
    self->setbyte_ = setbyte;
}

void
mementry_Init(mementry_s *self, addr_t begin, addr_t end, bool readonly,
              memdecode_f decode, void *opaque)
{
    init(self, begin, end, readonly, decode, NULL, NULL, opaque);
}

void
mementry_InitExt(mementry_s *self, addr_t begin, addr_t end, bool readonly,
                 memgetb_f getbyte, memsetb_f setbyte, void *opaque)
{
    init(self, begin, end, readonly, NULL, getbyte, setbyte, opaque);
}
