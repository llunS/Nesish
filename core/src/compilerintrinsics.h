#pragma once

// detect compiler
#ifdef _MSC_VER
#define NH_MSVC
#endif

// msvc intrinsics
#ifdef NH_MSVC
#define NH_VC_WARNING_PUSH __pragma(warning(push))
#define NH_VC_WARNING_DISABLE(no) __pragma(warning(disable : no))
#define NH_VC_WARNING_POP __pragma(warning(pop))
#else
#define NH_VC_WARNING_PUSH
#define NH_VC_WARNING_DISABLE(no)
#define NH_VC_WARNING_POP
#endif
