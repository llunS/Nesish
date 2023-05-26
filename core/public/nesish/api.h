
#ifndef NH_API_H
#define NH_API_H

#ifdef NESISH_STATIC_DEFINE
#  define NH_API
#  define NESISH_NO_EXPORT
#else
#  ifndef NH_API
#    ifdef Nesish_EXPORTS
        /* We are building this library */
#      define NH_API __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define NH_API __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef NESISH_NO_EXPORT
#    define NESISH_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef NESISH_DEPRECATED
#  define NESISH_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef NESISH_DEPRECATED_EXPORT
#  define NESISH_DEPRECATED_EXPORT NH_API NESISH_DEPRECATED
#endif

#ifndef NESISH_DEPRECATED_NO_EXPORT
#  define NESISH_DEPRECATED_NO_EXPORT NESISH_NO_EXPORT NESISH_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef NESISH_NO_DEPRECATED
#    define NESISH_NO_DEPRECATED
#  endif
#endif

#endif /* NH_API_H */
