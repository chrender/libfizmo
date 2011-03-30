
.PHONY : all locales-install clean distclean

include config.mk

TMPLIBDIR = libfizmotmp
PKG_DIR = $(INSTALL_PREFIX)/lib/pkgconfig
PKGFILE = $(PKG_DIR)/libfizmo.pc

all: libfizmo.a

libfizmo.a: src/tools/libtools.a src/interpreter/libinterpreter.a
	mkdir -p $(TMPLIBDIR) ; \
	cd $(TMPLIBDIR) ; \
	$(AR) x ../src/tools/libtools.a ; \
	$(AR) x ../src/interpreter/libinterpreter.a ; \
	$(AR) rcs ../libfizmo.a *.o ; \
	cd .. ; \
	rm -r $(TMPLIBDIR)

install: libfizmo.a
	mkdir -p $(INSTALL_PREFIX)/lib/fizmo
	cp libfizmo.a $(INSTALL_PREFIX)/lib/fizmo
	mkdir -p $(INSTALL_PREFIX)/include/fizmo/interpreter
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
	mkdir -p $(PKG_DIR)
	echo 'prefix=$(INSTALL_PREFIX)' >$(PKGFILE)
	echo 'exec_prefix=$${prefix}' >>$(PKGFILE)
	echo 'libdir=$${exec_prefix}/lib/fizmo' >>$(PKGFILE)
	echo 'includedir=$${prefix}/include/fizmo' >>$(PKGFILE)
	echo >>$(PKGFILE)
	echo 'Name: libfizmo' >>$(PKGFILE)
	echo 'Description: libfizmo' >>$(PKGFILE)
	echo 'Version: 0.7.0-b8' >>$(PKGFILE)
ifeq ($(DISABLE_LIBXML2),)
	echo 'Requires: libxml-2.0' >>$(PKGFILE)
else
	echo 'Requires:' >>$(PKGFILE)
endif
	echo 'Requires.private:' >>$(PKGFILE)
	echo 'Cflags: -I$(INSTALL_PREFIX)/include/fizmo ' >>$(PKGFILE)
	echo 'Libs: -L$(INSTALL_PREFIX)/lib/fizmo -lfizmo'  >>$(PKGFILE)
	echo >>$(PKGFILE)

clean::
	cd src/interpreter ; make clean
	cd src/tools ; make clean
	cd src/locales ; make clean

distclean:: clean
	rm -f libfizmo.a
	cd src/interpreter ; make distclean
	cd src/tools ; make distclean

src/tools/libtools.a::
	cd src/tools ; make

src/interpreter/libinterpreter.a::
	cd src/interpreter ; make

