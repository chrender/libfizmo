
localedir = $(datarootdir)/fizmo/locales

AM_CFLAGS =

noinst_LIBRARIES = libtools.a
libtools_a_SOURCES = ../locales/libfizmo_locales.c filesys.c filesys_c.c \
 i18n.c list.c stringmap.c tracelog.c types.c z_ucs.c

if ENABLE_TRACING
AM_CFLAGS += -DENABLE_TRACING=
endif

AM_CFLAGS += -DI18N_DEFAULT_SEARCH_PATH=\"$(localedir)\"

