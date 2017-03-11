SUBDIRS = data man po help src tests docs

@CODE_COVERAGE_RULES@

AM_DISTCHECK_CONFIGURE_FLAGS =	\
	--enable-gtk-doc	\
	--enable-introspection	\
	--enable-vala		\
	--enable-appstream-util

EXTRA_DIST =			\
	README			\
	HACKING			\
	latexila.doap		\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update

MAINTAINERCLEANFILES =		\
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	gtk-doc.make \
	INSTALL \
	m4/appstream-xml.m4 \
	m4/ax_append_compile_flags.m4 \
	m4/ax_append_flag.m4 \
	m4/ax_append_link_flags.m4 \
	m4/ax_check_compile_flag.m4 \
	m4/ax_check_link_flag.m4 \
	m4/ax_code_coverage.m4 \
	m4/ax_compiler_flags_cflags.m4 \
	m4/ax_compiler_flags_gir.m4 \
	m4/ax_compiler_flags_ldflags.m4 \
	m4/ax_compiler_flags.m4 \
	m4/ax_is_release.m4 \
	m4/ax_require_defined.m4 \
	m4/gsettings.m4 \
	m4/gtk-doc.m4 \
	m4/intltool.m4 \
	m4/introspection.m4 \
	m4/nls.m4 \
	m4/pkg.m4 \
	m4/yelp.m4

-include $(top_srcdir)/git.mk
