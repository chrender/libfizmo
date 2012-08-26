
AM_CFLAGS =
AM_CPPFLAGS =
LDADD =

noinst_LIBRARIES = libinterpreter.a
libinterpreter_a_SOURCES = babel.c blorb.c config.c fizmo.c hyphenation.c \
 iff.c math.c misc.c mt19937ar.c object.c output.c property.c routine.c \
 savegame.c sound.c stack.c streams.c table.c text.c undo.c variable.c \
 wordwrap.c zpu.c

if ENABLE_TRACING
AM_CFLAGS += -DENABLE_TRACING=
endif

if ENABLE_BABEL
AM_CPPFLAGS += -DUSE_LIBXML2
AM_CFLAGS += $(libxml2_CFLAGS)
LDADD += $(libxml2_LIBS)
else
AM_CFLAGS += -DDISABLE_BABEL=
endif

if ENABLE_STRICT_Z
AM_CFLAGS += -DSTRICT_Z=
endif

if ENABLE_SEGFAULT_ON_ERROR
AM_CFLAGS += -DTHROW_SIGFAULT_ON_ERROR=
endif

if ENABLE_FILELIST
libinterpreter_a_SOURCES += filelist.c
else
AM_CFLAGS += -DDISABLE_FILELIST=
endif

if ENABLE_BLOCKBUFFER
libinterpreter_a_SOURCES += blockbuf.c
else
AM_CFLAGS += -DDISABLE_BLOCKBUFFER=
endif

if ENABLE_COMMAND_HISTORY
libinterpreter_a_SOURCES += cmd_hst.c
else
AM_CFLAGS += -DDISABLE_COMMAND_HISTORY=
endif

if ENABLE_OUTPUT_HISTORY
libinterpreter_a_SOURCES += history.c
else
AM_CFLAGS += -DDISABLE_OUTPUT_HISTORY=
endif

if ENABLE_CONFIG_FILES
else
AM_CFLAGS += -DDISABLE_CONFIGFILES=
endif

if ENABLE_PREFIX_COMMANDS
else
AM_CFLAGS += -DDISABLE_PREFIX_COMMANDS=
endif

if ENABLE_DEBUGGER
libinterpreter_a_SOURCES += debugger.c
AM_CFLAGS += -DENABLE_DEBUGGER=
endif

