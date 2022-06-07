
#pragma once

#include "common/compiler_traits.hpp"

#ifdef LN_MSVC
#define LN_VC_WARNING_PUSH __pragma(warning(push))
#define LN_VC_WARNING_DISABLE(i_no) __pragma(warning(disable : i_no))
#define LN_VC_WARNING_POP __pragma(warning(pop))
#else
#define LN_VC_WARNING_PUSH
#define LN_VC_WARNING_DISABLE(i_no)
#define LN_VC_WARNING_POP
#endif
