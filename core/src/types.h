#pragma once

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

typedef unsigned int ubit;
#define UBITFMTX "%X"

typedef uint8_t u8; // MUST match NHByte
#define U8FMT "%" PRIu8
#define U8FMTX "%02" PRIX8

typedef int8_t i8;

typedef uint16_t u16;
#define U16FMTX "%04" PRIX16

typedef int16_t i16;

typedef size_t usize;
#define USIZEFMT "%zu"

typedef size_t cycle_t; // MUST match NHCycle

typedef u16 addr_t; // MUST match NHAddr
#define ADDRMAX ((u16)-1)
#define ADDRFMT "$" U16FMTX
