#
# This file is part of maemo-sharing-twitpic and is published under
# the GNU GPLv3.
#
# Copyright (C) 2010 Igalia, S.L.
#
# See the README file for more details.
#

plugins_LTLIBRARIES = libtwitpic.la

libtwitpic_la_SOURCES = plugin.c \
	twitpic.c twitpic.h \
	util.c util.h

libtwitpic_la_CFLAGS = $(EXTRA_CFLAGS)
libtwitpic_la_LIBADD = $(EXTRA_LIBS)
libtwitpic_la_LDFLAGS = -module -avoid-version -export-dynamic

MAINTAINERCLEANFILES = Makefile.in
