#pragma once

#include "nesish/nesish.h"

inline bool
IsDebugOn(NHDFlag flags, NHDFlag flag)
{
    return flags & flag;
}

inline bool
IsDebugOff(NHDFlag flags)
{
    return flags == NHD_DBG_OFF;
}

inline void
DebugOn(NHDFlag *flags, NHDFlag flag)
{
    *flags = (NHDFlag)(*flags | flag);
}

inline void
DebugOff(NHDFlag *flags, NHDFlag flag)
{
    *flags = (NHDFlag)(*flags & ~flag);
}
