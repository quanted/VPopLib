
#ifndef libvpop_EXPORT_H
#define libvpop_EXPORT_H

#ifdef LIBPORTCODE_STATIC_DEFINE
#  define libportcode_EXPORT
#  define LIBPORTCODE_NO_EXPORT
#else
#  ifndef libportcode_EXPORT
#    ifdef libportcode_EXPORTS
        /* We are building this library */
#      define libportcode_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define libportcode_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef LIBPORTCODE_NO_EXPORT
#    define LIBPORTCODE_NO_EXPORT 
#  endif
#endif

#ifndef LIBPORTCODE_DEPRECATED
#  define LIBPORTCODE_DEPRECATED __declspec(deprecated)
#endif

#ifndef LIBPORTCODE_DEPRECATED_EXPORT
#  define LIBPORTCODE_DEPRECATED_EXPORT libportcode_EXPORT LIBPORTCODE_DEPRECATED
#endif

#ifndef LIBPORTCODE_DEPRECATED_NO_EXPORT
#  define LIBPORTCODE_DEPRECATED_NO_EXPORT LIBPORTCODE_NO_EXPORT LIBPORTCODE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LIBPORTCODE_NO_DEPRECATED
#    define LIBPORTCODE_NO_DEPRECATED
#  endif
#endif

#endif /* libportcode_EXPORT_H */
