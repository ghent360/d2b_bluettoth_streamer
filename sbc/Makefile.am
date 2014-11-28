lib_LTLIBRARIES = libsbc.la
libsbc_la_SOURCES = \
    sbc.c           \
    sbc_primitives.c        \
    sbc_primitives_mmx.c    \
    sbc_primitives_armv6.c  \
    sbc_primitives_iwmmxt.c \
    sbc_primitives_neon.c   \
    sbc.h                   \
    sbc_primitives.h        \
    sbc_primitives_neon.h   \
    sbc_math.h              \
    sbc_primitives_iwmmxt.h \
    sbc_private.h           \
    sbc_primitives_armv6.h  \
    sbc_primitives_mmx.h    \
    sbc_tables.h

libsbc_la_CFLAGS = \
    -finline-functions \
    -fgcse-after-reload \
    -funswitch-loops \
    -funroll-loops
