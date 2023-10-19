#include "mapperbase.h"

#include "memory/vmem.h"
#include "cartridge/inesromaccessor.h"

void
mapperbase_Init(mapperbase_s *self, const inesromaccessor_s *accessor)
{
    self->romaccessor = accessor;
}

void
mapperbase_setFixedVhMirror(mapperbase_s *self, vmem_s *vmem)
{
    vmem_SetMirrorFixed(
        vmem, inesromaccessor_MirrorH(self->romaccessor) ? MM_H : MM_V);
}

void
mapperbase_unsetFixedVhMirror(mapperbase_s *self, vmem_s *vmem)
{
    (void)(self);
    vmem_UnsetMirror(vmem);
}
