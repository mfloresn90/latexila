@GNOME_CODE_COVERAGE_RULES@

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/src/liblatexila		\
	-I$(top_builddir)/src/liblatexila	\
	$(LATEXILA_CFLAGS)			\
	$(WARN_CFLAGS)

AM_LDFLAGS = $(WARN_LDFLAGS)

LDADD =	$(top_builddir)/src/liblatexila/liblatexila.la	\
	$(LATEXILA_LIBS)

build-tools-resources.c: build-tools.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/build-tools.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/build-tools.gresource.xml

BUILT_SOURCES = build-tools-resources.c

UNIT_TEST_PROGS = test-build-tools
test_build_tools_SOURCES = test-build-tools.c
nodist_test_build_tools_SOURCES = build-tools-resources.c

UNIT_TEST_PROGS += test-utils
test_utils_SOURCES = test-utils.c

noinst_PROGRAMS = $(UNIT_TEST_PROGS)
TESTS = $(UNIT_TEST_PROGS)

EXTRA_DIST =				\
	build-tools.gresource.xml	\
	build_tools_test.xml

CLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
