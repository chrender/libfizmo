
.PHONY : all install clean distclean

include config.mk

TMPLIBDIR = libfizmotmp

all: libfizmo.a libfizmo.mk

libfizmo.a: src/tools/libtools.a src/interpreter/libinterpreter.a
	mkdir -p $(TMPLIBDIR) ; \
	cd $(TMPLIBDIR) ; \
	$(AR) x ../src/tools/libtools.a ; \
	$(AR) x ../src/interpreter/libinterpreter.a ; \
	$(AR) rcs ../libfizmo.a *.o ; \
	cd .. ; \
	rm -r $(TMPLIBDIR)

install: libfizmo.a libfizmo.mk
	mkdir -p $(INSTALL_PREFIX)/lib/fizmo
	cp libfizmo.a $(INSTALL_PREFIX)/lib/fizmo
	mkdir -p $(INSTALL_PREFIX)/include/fizmo/interpreter
	cp libfizmo.mk $(INSTALL_PREFIX)/include/fizmo
	cp src/interpreter/*.h $(INSTALL_PREFIX)/include/fizmo/interpreter
	mkdir -p $(INSTALL_PREFIX)/include/fizmo/tools
	cp src/tools/*.h $(INSTALL_PREFIX)/include/fizmo/tools
	mkdir -p $(INSTALL_PREFIX)/include/fizmo/screen_interface
	cp src/screen_interface/*.h \
	  $(INSTALL_PREFIX)/include/fizmo/screen_interface
	cp src/screen_interface/*.cpp \
	  $(INSTALL_PREFIX)/include/fizmo/screen_interface
	cp -r src/sound_interface $(INSTALL_PREFIX)/include/fizmo/
	mkdir -p $(INSTALL_PREFIX)/share/fizmo/locales
	for l in `cd src/locales ; ls -d ??_??`; \
	do \
	  mkdir -p $(INSTALL_PREFIX)/share/fizmo/locales/$$l; \
	  cp src/locales/$$l/*.txt $(INSTALL_PREFIX)/share/fizmo/locales/$$l; \
	done

clean::
	cd src/interpreter ; make clean
	cd src/tools ; make clean
	cd src/locales ; make clean

distclean:: clean
	rm -f libfizmo.a libfizmo.mk
	cd src/interpreter ; make distclean
	cd src/tools ; make distclean

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
	
src/interpreter/libinterpreter.a::
	cd src/interpreter ; make

