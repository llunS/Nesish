#pragma once

#include "types.h"
#include "cpu/addrmode.h"

typedef struct cpu_s cpu_s;
typedef void (*instrcore_f)(cpu_s *cpu, u8 in, u8 *out);
typedef void (*instrfrm_f)(int idx, cpu_s *cpu, instrcore_f core, bool *done);

typedef struct instrdesc_s {
    instrcore_f Core;
    addrmode_e Addrmode;
    instrfrm_f Frm;
} instrdesc_s;
