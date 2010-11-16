
# Please read "INSTALL.txt" before modifying these values.

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra

ifneq ($(DESTDIR),)
INSTALL_PREFIX = $(DESTDIR)
else
#INSTALL_PREFIX = /usr/local
INSTALL_PREFIX = $(HOME)/opt/fizmo
endif

DEFAULT_PREFIX = /opt/local
DEFAULT_LIB_PREFIX = $(DEFAULT_PREFIX)/lib
DEFAULT_INC_PREFIX = $(DEFAULT_PREFIX)/include
LOCALE_SEARCH_PATH = $(INSTALL_PREFIX)/share/fizmo/locales

# libxml2 is required for babel metadata.
#DISABLE_LIBXML2 = 1
LIBXML2_INC_DIR = /usr/include/libxml2
LIBXML2_LIB_DIR = /usr/lib

# This adds an -O2 flag (usually okay):
ENABLE_OPTIMIZATION = 1

# If you're building a "dumb" interface like the CGI-interface (this
# runs the minizork-demo on the webpage, take a look at src/cgi) you
# may want to save memory and cpu by uncommenting the following lines.
# NOTE: Once disabled, these settings can only be re-enabled by
# rebuilding the library. It is recommended to leave the lines commented.
#DISABLE_BLOCKBUFFER = 1
#DISABLE_COMMAND_HISTORY = 1
#DISABLE_OUTPUT_HISTORY = 1

# Debug-Flags:

# Uncomment to fill your harddisk _very_ fast:
ENABLE_TRACING = 1

# Used for the strictz-test:
#ENABLE_STRICT_Z = 1

# Add GDB symbols, only useful for debuggong:
ENABLE_GDB_SYMBOLS = 1

# Throws sigfault on error for emergency backtrace (usually never needed):
#THROW_SIGFAULT_ON_ERROR = 1

