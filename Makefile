
.PHONY : all install clean distclean

include config.mk

TMPLIBDIR = libfizmotmp

all: libfizmo.a src/cell_interface/libcellif.a libfizmo.mk

libfizmo.a: src/tools/libtools.a src/interpreter/libinterpreter.a
	mkdir -p $(TMPLIBDIR) ; \
	cd $(TMPLIBDIR) ; \
	$(AR) x ../src/tools/libtools.a ; \
	$(AR) x ../src/interpreter/libinterpreter.a ; \
	$(AR) rcs ../libfizmo.a *.o ; \
	cd .. ; \
	rm -r $(TMPLIBDIR)

install: libfizmo.a src/cell_interface/libcellif.a libfizmo.mk libcellif.mk
	mkdir -p $(INSTALL_PREFIX)/lib/fizmo
	cp libfizmo.a $(INSTALL_PREFIX)/lib/fizmo
	mkdir -p $(INSTALL_PREFIX)/include/fizmo/interpreter
	cp libfizmo.mk $(INSTALL_PREFIX)/include/fizmo
	cp libcellif.mk $(INSTALL_PREFIX)/include/fizmo
	cp src/interpreter/*.h $(INSTALL_PREFIX)/include/fizmo/interpreter
	mkdir -p $(INSTALL_PREFIX)/include/fizmo/tools
	cp src/tools/*.h $(INSTALL_PREFIX)/include/fizmo/tools
	mkdir -p $(INSTALL_PREFIX)/include/fizmo/cell_interface
	cp src/cell_interface/*.h $(INSTALL_PREFIX)/include/fizmo/cell_interface
	cp src/cell_interface/libcellif.a $(INSTALL_PREFIX)/lib/fizmo
	cp -r src/screen_interface $(INSTALL_PREFIX)/include/fizmo/
	cp -r src/sound_interface $(INSTALL_PREFIX)/include/fizmo/
	mkdir -p $(INSTALL_PREFIX)/share/fizmo/locales
	for l in `cd src/locales ; ls -d ??_??`; \
	do \
	  mkdir -p $(INSTALL_PREFIX)/share/fizmo/locales/$$l; \
	  cp src/locales/$$l/* $(INSTALL_PREFIX)/share/fizmo/locales/$$l; \
	done

clean::
	cd src/interpreter ; make clean
	cd src/tools ; make clean
	cd src/cell_interface ; make clean
	cd src/locales ; make clean

distclean:: clean
	rm -f libfizmo.a ;
	cd src/interpreter ; make distclean
	cd src/tools ; make distclean
	cd src/cell_interface ; make distclean

src/tools/libtools.a::
	cd src/tools ; make

libfizmo.mk::
	echo > libfizmo.mk
	echo LIBFIZMO_INC_DIRS = -I$(INSTALL_PREFIX)/include/fizmo >>libfizmo.mk
	echo LIBFIZMO_LIB_DIRS = -L$(INSTALL_PREFIX)/lib/fizmo >> libfizmo.mk
	echo LIBFIZMO_LIBS = -lfizmo -lm >> libfizmo.mk
ifeq ($(DISABLE_LIBXML2),)
	echo >> libfizmo.mk
	echo LIBFIZMO_INC_DIRS += -I$(LIBXML2_INC_DIR) >> libfizmo.mk
	echo LIBFIZMO_LIB_DIRS += -L$(LIBXML2_LIB_DIR) >> libfizmo.mk
	echo LIBFIZMO_LIBS += -lxml2 >> libfizmo.mk
endif
	echo >> libfizmo.mk
	
libcellif.mk::
	echo > libcellif.mk
	echo LIBCELLIF_INC_DIRS = -I$(INSTALL_PREFIX)/include/fizmo >> libcellif.mk
	echo LIBCELLIF_LIB_DIRS = -L$(INSTALL_PREFIX)/lib/fizmo >> libcellif.mk
	echo LIBCELLIF_LIBS = -lcellif >> libcellif.mk
	echo >> libcellif.mk

src/interpreter/libinterpreter.a::
	cd src/interpreter ; make

src/cell_interface/libcellif.a::
	cd src/cell_interface ; make

