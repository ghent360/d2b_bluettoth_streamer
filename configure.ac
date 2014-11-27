AC_INIT([bt-a2dp], [1.0], [veffremov.ve@gmail.com])
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_CPP
AC_PROG_MKDIR_P

AC_CONFIG_HEADERS([config.h])

# Checks for libraries.
AC_CHECK_LIB([asound], [snd_pcm_close])
#AC_CHECK_LIB([gflags], [main])
#AC_CHECK_LIB([glog], [main])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([float.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([bzero clock_gettime gethostbyname getpagesize gettimeofday memchr memset select strtol strtoul strtoull])

PKG_PROG_PKG_CONFIG

DBUS_REQUIRED_VERSION=0.60
PKG_CHECK_MODULES(dbus, [dbus-1 >= $DBUS_REQUIRED_VERSION],,
	AC_MSG_ERROR([You need the DBus libraries (version 0.6 or better)]
	[http://www.freedesktop.org/wiki/Software_2fdbus])
)
AC_SUBST(dbus_CFLAGS)
AC_SUBST(dbus_LIBS)

LIBGLOG_REQUIRED_VERSION=0.3.3
PKG_CHECK_MODULES(libglog, [libglog >= $LIBGLOG_REQUIRED_VERSION],,
	AC_MSG_ERROR([You need the glog library (version 0.3.3 or better)]
	[google.com google glog])
)
AC_SUBST(libglog_CFLAGS)
AC_SUBST(libglog_LIBS)

AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
