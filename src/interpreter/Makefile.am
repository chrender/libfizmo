
# Makefile.am
#
# This file is part of fizmo.
#
# Copyright (c) 2009-2012 Christoph Ender.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


AM_CFLAGS =
AM_CPPFLAGS =
LDADD =

# Don't run ranlib directly, since in case OS X universal libraries are
# built, files processed by ranlib cannot be any longer processed by ar
# (which is required to assemble libfizmo later on).
RANLIB = true

noinst_LIBRARIES = libinterpreter.a
libinterpreter_a_SOURCES = babel.c blorb.c config.c fizmo.c hyphenation.c \
 iff.c mathemat.c misc.c mt19937ar.c object.c output.c property.c routine.c \
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

