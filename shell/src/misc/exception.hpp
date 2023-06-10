#pragma once

#ifdef SH_TGT_WEB
#define SH_NO_EXCEPTION
#endif

#ifndef SH_NO_EXCEPTION
#define SH_TRY try
#define SH_CATCH(x) catch (x)
#else
#define SH_TRY
#define SH_CATCH(x) if (false)
#endif
