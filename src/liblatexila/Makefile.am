@GNOME_CODE_COVERAGE_RULES@

noinst_LTLIBRARIES = liblatexila.la

liblatexila_la_CFLAGS =			\
	$(WARN_CFLAGS)			\
	$(CODE_COVERAGE_CFLAGS)		\
	-I$(top_builddir)/src/evince

liblatexila_la_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)
liblatexila_la_LIBADD = ../evince/libevince.la

liblatexila_headers =				\
	latexila.h				\
	latexila-build-job.h			\
	latexila-build-tool.h			\
	latexila-build-tools.h			\
	latexila-build-tools-default.h		\
	latexila-build-tools-personal.h		\
	latexila-build-view.h			\
	latexila-post-processor.h		\
	latexila-post-processor-all-output.h	\
	latexila-post-processor-latex.h		\
	latexila-post-processor-latexmk.h	\
	latexila-synctex.h			\
	latexila-templates-common.h		\
	latexila-templates-default.h		\
	latexila-templates-dialogs.h		\
	latexila-templates-manage-dialog.h	\
	latexila-templates-personal.h		\
	latexila-types.h			\
	latexila-utils.h

liblatexila_sources =				\
	latexila-build-job.c			\
	latexila-build-tool.c			\
	latexila-build-tools.c			\
	latexila-build-tools-default.c		\
	latexila-build-tools-personal.c		\
	latexila-build-view.c			\
	latexila-post-processor.c		\
	latexila-post-processor-all-output.c	\
	latexila-post-processor-latex.c		\
	latexila-post-processor-latexmk.c	\
	latexila-synctex.c			\
	latexila-templates-common.c		\
	latexila-templates-default.c		\
	latexila-templates-dialogs.c		\
	latexila-templates-manage-dialog.c	\
	latexila-templates-personal.c		\
	latexila-utils.c

liblatexila_built_sources =	\
	latexila-enum-types.c	\
	latexila-enum-types.h

liblatexila_la_SOURCES = 	\
	$(liblatexila_headers)	\
	$(liblatexila_sources)

nodist_liblatexila_la_SOURCES = $(liblatexila_built_sources)

BUILT_SOURCES = $(liblatexila_built_sources)

ENUM_TYPES = $(liblatexila_headers)

latexila-enum-types.h: latexila-enum-types.h.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template latexila-enum-types.h.template $(ENUM_TYPES)) > $@

latexila-enum-types.c: latexila-enum-types.c.template $(ENUM_TYPES) $(GLIB_MKENUMS)
	$(AM_V_GEN) (cd $(srcdir) && $(GLIB_MKENUMS) --template latexila-enum-types.c.template $(ENUM_TYPES)) > $@

EXTRA_DIST =				\
	latexila-enum-types.c.template	\
	latexila-enum-types.h.template

CLEANFILES = $(BUILT_SOURCES)

noinst_DATA =

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
Latexila_gir_INCLUDES = Gtk-3.0
Latexila_gir_LIBS = liblatexila.la
Latexila_gir_FILES = $(liblatexila_la_SOURCES) $(nodist_liblatexila_la_SOURCES)

noinst_DATA += Latexila.gir Latexila.typelib
CLEANFILES += Latexila.gir Latexila.typelib

endif # HAVE_INTROSPECTION

if ENABLE_VAPIGEN
include $(VAPIGEN_MAKEFILE)

latexila.vapi: Latexila.gir

VAPIGEN_VAPIS = latexila.vapi

latexila_vapi_DEPS = gio-2.0 gtk+-3.0
latexila_vapi_FILES = Latexila.gir

noinst_DATA += latexila.vapi
CLEANFILES += latexila.vapi

EXTRA_DIST = latexila.vapi

endif # ENABLE_VAPIGEN

-include $(top_srcdir)/git.mk
