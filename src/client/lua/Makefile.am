CC = $(CC_FOR_BUILD)
CPPFLAGS = $(CPPFLAGS_FOR_BUILD)
CFLAGS = $(CFLAGS_FOR_BUILD)
LDFLAGS = $(LDFLAGS_FOR_BUILD)
LIBS = $(LIBS_FOR_BUILD)
EXEEXT = $(EXEEXT_FOR_BUILD)

AM_CPPFLAGS=-I$(top_srcdir)/src/include -I$(top_srcdir)/src/client -I$(top_builddir)/src/client

EXTRA_DIST = io.inc
CLEANFILES = gen.inc EIBConnection.lua result.inc

result.inc : $(top_srcdir)/src/common/eibloadresult.h
	cat $<  |grep IMG_ |sed -e 's/#define \(IMG_[A-Z_0-9]\+\)  \+\([0-9]\+\)/\1 = \2/g' > $@

noinst_PROGRAMS = gen

gen_SOURCES=gen.c arg.def def.def

pkgdata_DATA = EIBConnection.lua

gen.inc : gen$(EXEEXT)
	./$< > $@

EIBConnection.lua : io.inc gen.inc result.inc
	cat $^ > $@
