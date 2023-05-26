
#ifndef NB_API_H
#define NB_API_H

#ifdef NESISHBASE_STATIC_DEFINE
#  define NB_API
#  define NESISHBASE_NO_EXPORT
#else
#  ifndef NB_API
#    ifdef NesishBase_EXPORTS
        /* We are building this library */
#      define NB_API __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define NB_API __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef NESISHBASE_NO_EXPORT
#    define NESISHBASE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef NESISHBASE_DEPRECATED
#  define NESISHBASE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef NESISHBASE_DEPRECATED_EXPORT
#  define NESISHBASE_DEPRECATED_EXPORT NB_API NESISHBASE_DEPRECATED
#endif

#ifndef NESISHBASE_DEPRECATED_NO_EXPORT
#  define NESISHBASE_DEPRECATED_NO_EXPORT NESISHBASE_NO_EXPORT NESISHBASE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef NESISHBASE_NO_DEPRECATED
#    define NESISHBASE_NO_DEPRECATED
#  endif
#endif

#endif /* NB_API_H */
