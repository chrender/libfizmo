
AUTOMAKE_OPTIONS = -Wno-override

tmplibdir = libfizmotmp
localedir = $(datarootdir)/fizmo/locales

if FIZMO_BUILD_PREFIX_EMPTY
  fizmo_build_prefix=$(DESTDIR)$(prefix)
endif

PKG_DIR = $(fizmo_build_prefix)/lib/pkgconfig
PKGFILE = $(PKG_DIR)/libfizmo.pc

SUBDIRS = src/tools src/interpreter .
noinst_LIBRARIES = libfizmo.a

libfizmo.a::
	cd src/tools ; make
	cd src/interpreter ; make
	mkdir -p "$(tmplibdir)" ; \
	cd "$(tmplibdir)" ; \
	"$(AR)" x ../src/tools/libtools.a ; \
	"$(AR)" x ../src/interpreter/libinterpreter.a ; \
	"$(AR)" $(ARFLAGS) ../libfizmo.a *.o ; \
	"$(RANLIB)" ../libfizmo.a ; \
	cd .. ; \
	rm -r "$(tmplibdir)"

install-dev:: libfizmo.a
	mkdir -p "$(fizmo_build_prefix)/lib/fizmo"
	cp libfizmo.a "$(fizmo_build_prefix)/lib/fizmo"
	mkdir -p "$(fizmo_build_prefix)/include/fizmo/interpreter"
	cp src/interpreter/*.h "$(fizmo_build_prefix)/include/fizmo/interpreter"
	mkdir -p "$(fizmo_build_prefix)/include/fizmo/tools"
	cp src/tools/*.h "$(fizmo_build_prefix)/include/fizmo/tools"
	mkdir -p "$(fizmo_build_prefix)/include/fizmo/screen_interface"
	cp src/screen_interface/*.h \
	  "$(fizmo_build_prefix)/include/fizmo/screen_interface"
	cp src/screen_interface/*.cpp \
	  "$(fizmo_build_prefix)/include/fizmo/screen_interface"
	cp -r src/sound_interface "$(fizmo_build_prefix)/include/fizmo/"
	cp -r src/filesys_interface "$(fizmo_build_prefix)/include/fizmo/"
	cp -r src/blorb_interface "$(fizmo_build_prefix)/include/fizmo/"
	mkdir -p "$(PKG_DIR)"
	echo 'prefix=$(fizmo_build_prefix)' >"$(PKGFILE)"
	echo 'exec_prefix=$${prefix}' >>"$(PKGFILE)"
	echo 'libdir=$${exec_prefix}/lib/fizmo' >>"$(PKGFILE)"
	echo 'includedir=$${prefix}/include/fizmo' >>"$(PKGFILE)"
	echo >>"$(PKGFILE)"
	echo 'Name: libfizmo' >>"$(PKGFILE)"
	echo 'Description: libfizmo' >>"$(PKGFILE)"
	echo 'Version: 0.7.4' >>"$(PKGFILE)"
	echo "Requires: $(LIBFIZMO_REQS)" >>"$(PKGFILE)"
	echo 'Requires.private:' >>"$(PKGFILE)"
	echo 'Cflags: -I$(fizmo_build_prefix)/include/fizmo $(LIBXML2_NONPKG_CFLAGS)' >>"$(PKGFILE)"
	echo 'Libs: -L$(fizmo_build_prefix)/lib/fizmo -lfizmo $(LIBXML2_NONPKG_LIBS)'  >>"$(PKGFILE)"
	echo >>"$(PKGFILE)"

install-locales::
	mkdir -p "$(DESTDIR)$(localedir)"
	for l in `cd src/locales ; ls -d ??_??`; \
	do \
	mkdir -p "$(DESTDIR)$(localedir)/$$l" ; \
	cp src/locales/$$l/*.txt "$(DESTDIR)$(localedir)/$$l" ; \
	done

