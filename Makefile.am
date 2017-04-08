
# Makefile.am
#
# This file is part of fizmo.
#
# Copyright (c) 2009-2017 Christoph Ender.
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



AUTOMAKE_OPTIONS = -Wno-override

tmplibdir = libfizmotmp
localedir = $(datarootdir)/fizmo/locales

if FIZMO_DIST_BUILD
  dev_prefix=$(FIZMO_DIST_BUILD_PREFIX)
else
  dev_prefix=$(prefix)
endif

fizmo_build_prefix=$(dev_prefix)

pkg_dir = $(DESTDIR)$(dev_prefix)/lib/pkgconfig
pkgfile = $(pkg_dir)/libfizmo.pc

SUBDIRS = src/tools src/interpreter .
noinst_LIBRARIES = libfizmo.a

# Object files are stored in pre-defined order into the static library in
# order to make the build reproducible for Debian's ReproducibleBuilds
# project, see https://wiki.debian.org/ReproducibleBuilds
# Patch provided by Sascha Steinbiss <satta@debian.org>.
libfizmo.a::
	cd src/tools ; make
	cd src/interpreter ; make
	mkdir -p "$(tmplibdir)" ; \
	cd "$(tmplibdir)" ; \
	"$(AR)" x ../src/tools/libtools.a ; \
	"$(AR)" x ../src/interpreter/libinterpreter.a ; \
	find . -name '*.o' -print0 | LC_ALL=C sort -z | xargs -0 "$(AR)" $(ARFLAGS) ../libfizmo.a ; \
	"$(RANLIB)" ../libfizmo.a ; \
	cd .. ; \
	rm -r "$(tmplibdir)"

install-dev:: libfizmo.a
	mkdir -p "$(DESTDIR)$(dev_prefix)/lib/fizmo"
	cp libfizmo.a "$(DESTDIR)$(dev_prefix)/lib/fizmo"
	mkdir -p "$(DESTDIR)$(dev_prefix)/include/fizmo/interpreter"
	cp src/interpreter/*.h \
	  "$(DESTDIR)$(dev_prefix)/include/fizmo/interpreter"
	mkdir -p "$(DESTDIR)$(dev_prefix)/include/fizmo/tools"
	cp src/tools/*.h "$(DESTDIR)$(dev_prefix)/include/fizmo/tools"
	mkdir -p "$(DESTDIR)$(dev_prefix)/include/fizmo/screen_interface"
	cp src/screen_interface/*.h \
	  "$(DESTDIR)$(dev_prefix)/include/fizmo/screen_interface"
	cp src/screen_interface/*.cpp \
	  "$(DESTDIR)$(dev_prefix)/include/fizmo/screen_interface"
	cp -r src/sound_interface "$(DESTDIR)$(dev_prefix)/include/fizmo/"
	cp -r src/filesys_interface "$(DESTDIR)$(dev_prefix)/include/fizmo/"
	cp -r src/blorb_interface "$(DESTDIR)$(dev_prefix)/include/fizmo/"
	mkdir -p "$(pkg_dir)"
	echo 'prefix=$(dev_prefix)' >"$(pkgfile)"
	echo 'exec_prefix=$${prefix}' >>"$(pkgfile)"
	echo 'libdir=$${exec_prefix}/lib/fizmo' >>"$(pkgfile)"
	echo 'includedir=$${prefix}/include/fizmo' >>"$(pkgfile)"
	echo >>"$(pkgfile)"
	echo 'Name: libfizmo' >>"$(pkgfile)"
	echo 'Description: libfizmo' >>"$(pkgfile)"
	echo 'Version: 0.7.14' >>"$(pkgfile)"
	echo "Requires: $(LIBFIZMO_REQS)" >>"$(pkgfile)"
	echo 'Requires.private:' >>"$(pkgfile)"
	echo 'Cflags: -I$(dev_prefix)/include/fizmo $(LIBXML2_NONPKG_CFLAGS)' >>"$(pkgfile)"
	echo 'Libs: -L$(dev_prefix)/lib/fizmo -lfizmo $(LIBXML2_NONPKG_LIBS) -lm'  >>"$(pkgfile)"
	echo >>"$(pkgfile)"

install-data-local::
	mkdir -p "$(DESTDIR)$(localedir)"
	for l in `cd src/locales ; ls -d ??_??`; \
	do \
	mkdir -p "$(DESTDIR)$(localedir)/$$l" ; \
	cp src/locales/$$l/*.txt "$(DESTDIR)$(localedir)/$$l" ; \
	done

# About "-rmdir": make ignores errors of commands which are prefixed with
# a - sign. rmdir may fail in clean-dev in case other libs have installed
# development files, or in case clean-dev has been run before.
clean-dev::
	-rm    "$(DESTDIR)$(dev_prefix)/lib/fizmo/libfizmo.a"
	-rmdir "$(DESTDIR)$(dev_prefix)/lib/fizmo"
	rm -rf "$(DESTDIR)$(dev_prefix)/include/fizmo/interpreter"
	rm -rf "$(DESTDIR)$(dev_prefix)/include/fizmo/tools"
	rm -rf "$(DESTDIR)$(dev_prefix)/include/fizmo/sound_interface"
	rm -rf "$(DESTDIR)$(dev_prefix)/include/fizmo/filesys_interface"
	rm -rf "$(DESTDIR)$(dev_prefix)/include/fizmo/blorb_interface"
	-rm    "$(DESTDIR)$(dev_prefix)/include/fizmo/screen_interface/ScreenInterface.cpp"
	-rm    "$(DESTDIR)$(dev_prefix)/include/fizmo/screen_interface/ScreenInterface.h"
	-rm    "$(DESTDIR)$(dev_prefix)/include/fizmo/screen_interface/screen_interface.h"
	-rmdir "$(DESTDIR)$(dev_prefix)/include/fizmo/screen_interface"
	-rmdir "$(DESTDIR)$(dev_prefix)/include/fizmo"
	-rm    "$(pkgfile)"

