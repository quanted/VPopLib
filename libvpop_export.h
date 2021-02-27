
#ifndef libvpop_EXPORT_H
#define libvpop_EXPORT_H

#ifdef LIBVPOP_STATIC_DEFINE
#  define libvpop_EXPORT
#  define LIBVPOP_NO_EXPORT
#else
#ifdef _WIN32
#  ifndef libvpop_EXPORT
#    ifdef libvpop_EXPORTS
        /* We are building this library */
#      define libvpop_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define libvpop_EXPORT __declspec(dllimport)
#    endif
#  endif
#else
#  define libvpop_EXPORT
#endif

#  ifndef LIBVPOP_NO_EXPORT
#    define LIBVPOP_NO_EXPORT 
#  endif
#endif

#ifndef LIBVPOP_DEPRECATED
#  define LIBVPOP_DEPRECATED __declspec(deprecated)
#endif

#ifndef LIBVPOP_DEPRECATED_EXPORT
#  define LIBVPOP_DEPRECATED_EXPORT libvpop_EXPORT LIBVPOP_DEPRECATED
#endif

#ifndef LIBVPOP_DEPRECATED_NO_EXPORT
#  define LIBVPOP_DEPRECATED_NO_EXPORT LIBVPOP_NO_EXPORT LIBVPOP_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LIBVPOP_NO_DEPRECATED
#    define LIBVPOP_NO_DEPRECATED
#  endif
#endif

#endif /* libvpop_EXPORT_H */
