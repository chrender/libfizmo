
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra

ifneq ($(DESTDIR),)
INSTALL_PREFIX = $(DESTDIR)
else
INSTALL_PREFIX = /opt/local
endif

# Uncomment to install binaries to $(INSTALL_PREFIX)/$(FIZMO_BIN_DIR).
FIZMO_BIN_DIR = bin


# -----
# General settings:
ENABLE_OPTIMIZATION = 1
#ENABLE_TRACING = 1
#ENABLE_GDB_SYMBOLS = 1
# -----



# -----
# Settings for libfizmo:
LOCALE_SEARCH_PATH = $(INSTALL_PREFIX)/share/fizmo/locales
#ENABLE_STRICT_Z = 1
#THROW_SIGFAULT_ON_ERROR = 1
#DISABLE_BABEL = 1
#DISABLE_FILELIST = 1
#DISABLE_BLOCKBUFFER = 1
#DISABLE_COMMAND_HISTORY = 1
#DISABLE_OUTPUT_HISTORY = 1
#DISABLE_CONFIGFILES = 1

# If libxml2 may be found using pkg-config -- may be tested by executing
# command "pkg-config --modversion libxml-2.0" -- fizmo will automatically
# find the required files using the following three lines:
LIBFIZMO_REQS = libxml-2.0
LIBXML2_PKG_CFLAGS = $(shell pkg-config --cflags libxml-2.0)
LIBXML2_PKG_LIBS = $(shell pkg-config --libs libxml-2.0)

# In case "pkg-config --modversion libxml-2.0" does not work, use the
# following two lines (and adapt locations if necessary):
LIBXML2_NONPKG_CFLAGS =
LIBXML2_NONPKG_LIBS =

# In case libxml2 is not available at all, uncomment "DISABLE_BABEL=1" above.
# -----


