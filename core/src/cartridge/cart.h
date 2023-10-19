#pragma once

#include "nesish/nesish.h"

typedef struct mmem_s mmem_s;
typedef struct vmem_s vmem_s;

typedef struct cart_s {
    void (*Deinit)(void *impl);

    NHErr (*Validate)(const void *impl);

    void (*Powerup)(void *impl);
    void (*Reset)(void *impl);

    void (*MapMemory)(void *impl, mmem_s *mmem, vmem_s *vmem);
    void (*UnmapMemory)(void *impl, mmem_s *mmem, vmem_s *vmem);

    void *Impl;
} cart_s;
