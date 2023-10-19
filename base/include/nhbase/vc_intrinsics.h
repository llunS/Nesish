#pragma once

#include "nhbase/compiler_traits.h"

#ifdef NB_MSVC
#define NB_VC_WARNING_PUSH __pragma(warning(push))
#define NB_VC_WARNING_DISABLE(no) __pragma(warning(disable : no))
#define NB_VC_WARNING_POP __pragma(warning(pop))
#else
#define NB_VC_WARNING_PUSH
#define NB_VC_WARNING_DISABLE(no)
#define NB_VC_WARNING_POP
#endif
