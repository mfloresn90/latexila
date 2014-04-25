AM_CPPFLAGS = $(WARN_CFLAGS)

noinst_LTLIBRARIES = liblatexila.la

liblatexila_la_SOURCES = 			\
	latexila-build-tools.c			\
	latexila-build-tools.h			\
	latexila-build-tools-default.c		\
	latexila-build-tools-default.h		\
	latexila-build-tools-personal.c		\
	latexila-build-tools-personal.h		\
	latexila-post-processor.c		\
	latexila-post-processor.h		\
	latexila-post-processor-all-output.c	\
	latexila-post-processor-all-output.h	\
	latexila-types.h

CLEANFILES =

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_SCANNER_ARGS =	\
	-I$(top_srcdir)		\
	-I$(top_builddir)	\
	--warn-all

INTROSPECTION_GIRS = Latexila.gir

Latexila.gir: liblatexila.la
Latexila_gir_NAMESPACE = Latexila
Latexila_gir_INCLUDES = Gio-2.0
Latexila_gir_LIBS = liblatexila.la
Latexila_gir_FILES = $(liblatexila_la_SOURCES)

noinst_DATA = Latexila.gir Latexila.typelib

CLEANFILES += Latexila.gir Latexila.typelib
endif

-include $(top_srcdir)/git.mk
