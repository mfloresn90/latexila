AM_CPPFLAGS =				\
	-I$(top_srcdir)			\
	-I$(top_srcdir)/src/liblatexila	\
	-DSRCDIR=\""$(ABS_SRCDIR)"\"	\
	$(WARN_CFLAGS)

LDADD = $(top_builddir)/src/liblatexila/liblatexila.la

UNIT_TEST_PROGS = test-build-tools
test_build_tools_SOURCES = test-build-tools.c

noinst_PROGRAMS = $(UNIT_TEST_PROGS)
TESTS = $(UNIT_TEST_PROGS)

EXTRA_DIST =	\
	build_tools_test.xml

-include $(top_srcdir)/git.mk
