
.PHONY : clean

include ../../config.mk

ifneq ($(ENABLE_GDB_SYMBOLS),)
override CFLAGS += -g
endif

ifneq ($(ENABLE_OPTIMIZATION),)
override CFLAGS += -O2
endif

ifneq ($(ENABLE_TRACING),)
override CFLAGS += -DENABLE_TRACING=
endif

libcellif.a: cell_interface.o
	$(AR) rcs libcellif.a cell_interface.o

cell_interface.o: cell_interface.h cell_interface.c
	$(CC) $(CFLAGS) -c cell_interface.c

clean:
	rm -f *.o

distclean: clean
	rm -f libcellif.a

