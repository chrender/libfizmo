
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra

ifneq ($(DESTDIR),)
INSTALL_PREFIX = $(DESTDIR)
else
#INSTALL_PREFIX = /usr/local
INSTALL_PREFIX = $(HOME)/opt/fizmo
endif

# Uncomment to install binaries to $(INSTALL_PREFIX)/$(FIZMO_BIN_DIR).
#FIZMO_BIN_DIR = bin


# General:
ENABLE_OPTIMIZATION = 1
ENABLE_TRACING = 1
#ENABLE_GDB_SYMBOLS = 1


# libfizmo:
LOCALE_SEARCH_PATH = $(INSTALL_PREFIX)/share/fizmo/locales
#ENABLE_STRICT_Z = 1
#THROW_SIGFAULT_ON_ERROR = 1
# In case libxml2 is not available, uncomment "DISABLE_BABEL=1" below.
LIBFIZMO_REQS = libxml-2.0
LIBXML2_PKG_CFLAGS = $(shell pkg-config --cflags libxml-2.0)
LIBXML2_PKG_LIBS = $(shell pkg-config --libs libxml-2.0)
LIBXML2_NONPKG_CFLAGS =
LIBXML2_NONPKG_LIBS =
#DISABLE_BABEL = 1
#DISABLE_FILELIST = 1
#DISABLE_BLOCKBUFFER = 1
#DISABLE_COMMAND_HISTORY = 1
#DISABLE_OUTPUT_HISTORY = 1
#DISABLE_CONFIGFILES = 1

