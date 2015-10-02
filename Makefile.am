
# Makefile.am
#
# This file is part of fizmo.
#
# Copyright (c) 2009-2015 Christoph Ender.
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

fizmo_build_prefix=$(DEV_BUILD_PREFIX)

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
	echo 'Version: 0.7.10' >>"$(PKGFILE)"
	echo "Requires: $(LIBFIZMO_REQS)" >>"$(PKGFILE)"
	echo 'Requires.private:' >>"$(PKGFILE)"
	echo 'Cflags: -I$(fizmo_build_prefix)/include/fizmo $(LIBXML2_NONPKG_CFLAGS)' >>"$(PKGFILE)"
	echo 'Libs: -L$(fizmo_build_prefix)/lib/fizmo -lfizmo $(LIBXML2_NONPKG_LIBS)'  >>"$(PKGFILE)"
	echo >>"$(PKGFILE)"

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
	-rm    "$(fizmo_build_prefix)/lib/fizmo/libfizmo.a"
	-rmdir "$(fizmo_build_prefix)/lib/fizmo"
	rm -rf "$(fizmo_build_prefix)/include/fizmo/interpreter"
	rm -rf "$(fizmo_build_prefix)/include/fizmo/tools"
	rm -rf "$(fizmo_build_prefix)/include/fizmo/sound_interface"
	rm -rf "$(fizmo_build_prefix)/include/fizmo/filesys_interface"
	rm -rf "$(fizmo_build_prefix)/include/fizmo/blorb_interface"
	-rm    "$(fizmo_build_prefix)/include/fizmo/screen_interface/ScreenInterface.cpp"
	-rm    "$(fizmo_build_prefix)/include/fizmo/screen_interface/ScreenInterface.h"
	-rm    "$(fizmo_build_prefix)/include/fizmo/screen_interface/screen_interface.h"
	-rmdir "$(fizmo_build_prefix)/include/fizmo/screen_interface"
	-rmdir "$(fizmo_build_prefix)/include/fizmo"
	-rmdir "$(fizmo_build_prefix)/include"
	-rm    "$(PKGFILE)"
	-rmdir "$(PKG_DIR)"
	-rmdir "$(fizmo_build_prefix)/lib"

