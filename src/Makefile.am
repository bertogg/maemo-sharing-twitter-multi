#
# This file is part of sharing-twitter-multi and is published under
# the GNU LGPLv3.
#
# Copyright (C) 2010 Igalia, S.L.
#
# See the README file for more details.
#

plugins_LTLIBRARIES = libtwittermulti.la

libtwittermulti_la_SOURCES = plugin.c \
	twitmulti.c twitmulti.h \
	util.c util.h

libtwittermulti_la_CFLAGS = $(EXTRA_CFLAGS)
libtwittermulti_la_LIBADD = $(EXTRA_LIBS)
libtwittermulti_la_LDFLAGS = -module -avoid-version -export-dynamic

MAINTAINERCLEANFILES = Makefile.in
