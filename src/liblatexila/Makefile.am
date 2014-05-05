@GNOME_CODE_COVERAGE_RULES@

noinst_LTLIBRARIES = liblatexila.la

liblatexila_la_CFLAGS = $(WARN_CFLAGS) $(CODE_COVERAGE_CFLAGS)
liblatexila_la_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)

liblatexila_headers =				\
	latexila-build-job.h			\
	latexila-build-tool.h			\
	latexila-build-tools.h			\
	latexila-build-tools-default.h		\
	latexila-build-tools-personal.h		\
	latexila-post-processor.h		\
	latexila-post-processor-all-output.h	\
	latexila-types.h

BUILT_SOURCES =			\
	latexila-enum-types.c	\
	latexila-enum-types.h

liblatexila_la_SOURCES = 			\
	latexila-build-job.c			\
	latexila-build-tool.c			\
	latexila-build-tools.c			\
	latexila-build-tools-default.c		\
	latexila-build-tools-personal.c		\
	latexila-post-processor.c		\
	latexila-post-processor-all-output.c	\
	$(liblatexila_headers)			\
	$(BUILT_SOURCES)

ENUM_TYPES = $(liblatexila_headers)

latexila-enum-types.h: latexila-enum-types.h.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template latexila-enum-types.h.template $(ENUM_TYPES)) > $@

latexila-enum-types.c: latexila-enum-types.c.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template latexila-enum-types.c.template $(ENUM_TYPES)) > $@

EXTRA_DIST =				\
	latexila-enum-types.c.template	\
	latexila-enum-types.h.template

noinst_DATA =
CLEANFILES =

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_SCANNER_ARGS =	\
	-I$(top_srcdir)		\
	-I$(top_builddir)	\
	--c-include=latexila.h	\
	--warn-all

INTROSPECTION_GIRS = Latexila.gir

Latexila.gir: liblatexila.la
Latexila_gir_NAMESPACE = Latexila
Latexila_gir_INCLUDES = Gio-2.0
Latexila_gir_LIBS = liblatexila.la
Latexila_gir_FILES = $(liblatexila_la_SOURCES)

noinst_DATA += Latexila.gir Latexila.typelib
CLEANFILES += Latexila.gir Latexila.typelib

endif # HAVE_INTROSPECTION

if ENABLE_VAPIGEN
include $(VAPIGEN_MAKEFILE)

latexila.vapi: Latexila.gir

VAPIGEN_VAPIS = latexila.vapi

latexila_vapi_DEPS = gio-2.0
latexila_vapi_FILES = Latexila.gir

noinst_DATA += latexila.vapi
CLEANFILES += latexila.vapi

endif # ENABLE_VAPIGEN

-include $(top_srcdir)/git.mk