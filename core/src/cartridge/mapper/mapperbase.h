#pragma once

typedef struct inesromaccessor_s inesromaccessor_s;
typedef struct vmem_s vmem_s;

typedef struct mapperbase_s {
    const inesromaccessor_s *romaccessor;
} mapperbase_s;

void
mapperbase_Init(mapperbase_s *self, const inesromaccessor_s *accessor);

void
mapperbase_setFixedVhMirror(mapperbase_s *self, vmem_s *vmem);
void
mapperbase_unsetFixedVhMirror(mapperbase_s *self, vmem_s *vmem);
