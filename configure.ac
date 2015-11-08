#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Init Autoconf
AC_PREREQ([2.64])

AC_INIT([LaTeXila],
        [3.18.1],
        [https://bugzilla.gnome.org/enter_bug.cgi?product=latexila],
	[latexila],
	[https://wiki.gnome.org/Apps/LaTeXila])

AC_CONFIG_SRCDIR([src/liblatexila/latexila.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Init Automake
AM_INIT_AUTOMAKE([1.12.5 foreign tar-ustar no-dist-gzip dist-xz -Wno-portability])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

# Init Libtool
LT_PREREQ([2.2])
LT_INIT

# Required dependencies versions
# Update also the --target-glib option in src/Makefile.am.
GLIB_REQUIRED_VERSION="2.40"
GTK_REQUIRED_VERSION="3.14"
GTKSOURCEVIEW_REQUIRED_VERSION="3.18"
GSPELL_REQUIRED_VERSION="0.1"
VALA_REQUIRED_VERSION="0.26"
GEE_REQUIRED_VERSION="0.10"

AC_SUBST([GLIB_REQUIRED_VERSION])
AC_SUBST([GTK_REQUIRED_VERSION])
AC_SUBST([GTKSOURCEVIEW_REQUIRED_VERSION])
AC_SUBST([GSPELL_REQUIRED_VERSION])
AC_SUBST([VALA_REQUIRED_VERSION])
AC_SUBST([GEE_REQUIRED_VERSION])

# Some directories

# adl_RECURSIVE_EVAL(VALUE, RESULT)
# =================================
# Interpolate the VALUE in loop until it doesn't change,
# and set the result to $RESULT.
# WARNING: It's easy to get an infinite loop with some unsane input.
# For example ${datadir} becomes ${datarootdir}, and then ${prefix}/share, and
# finally ${prefix} is replaced by the prefix.
AC_DEFUN([adl_RECURSIVE_EVAL],
[_lcl_receval="$1"
$2=`(test "x$prefix" = xNONE && prefix="$ac_default_prefix"
     test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
     _lcl_receval_old=''
     while test "[$]_lcl_receval_old" != "[$]_lcl_receval"; do
       _lcl_receval_old="[$]_lcl_receval"
       eval _lcl_receval="\"[$]_lcl_receval\""
     done
     echo "[$]_lcl_receval")`])

adl_RECURSIVE_EVAL(["${datadir}/latexila"], [DATA_DIR])
AC_DEFINE_UNQUOTED([DATA_DIR],
                   ["${DATA_DIR}"],
		   [latexila data directory])

adl_RECURSIVE_EVAL(["${datadir}/icons/hicolor"], [ICONS_DIR])
AC_DEFINE_UNQUOTED([ICONS_DIR],
                   ["${ICONS_DIR}"],
		   [icons directory])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_PATH_PROG([GLIB_COMPILE_RESOURCES], [glib-compile-resources])
AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
AC_PATH_PROG([GDBUS_CODEGEN], [gdbus-codegen])
AM_PROG_VALAC([${VALA_REQUIRED_VERSION}],
              [found_vala=true]
	      [found_vala=false])

# Checks for dependencies

if ! pkg-config --atleast-version=${GLIB_REQUIRED_VERSION} glib-2.0; then
	AC_MSG_ERROR([glib-2.0 >= ${GLIB_REQUIRED_VERSION} required.])
fi

if ! pkg-config --atleast-version=${GLIB_REQUIRED_VERSION} gio-2.0; then
	AC_MSG_ERROR([gio-2.0 >= ${GLIB_REQUIRED_VERSION} required.])
fi

if ! pkg-config --atleast-version=${GTK_REQUIRED_VERSION} gtk+-3.0; then
	AC_MSG_ERROR([gtk+-3.0 >= ${GTK_REQUIRED_VERSION} required.])
fi

if ! pkg-config --atleast-version=${GTKSOURCEVIEW_REQUIRED_VERSION} gtksourceview-3.0; then
	AC_MSG_ERROR([gtksourceview-3.0 >= ${GTKSOURCEVIEW_REQUIRED_VERSION} required.])
fi

if ! pkg-config --atleast-version=${GSPELL_REQUIRED_VERSION} gspell-1; then
	AC_MSG_ERROR([gspell-1 ${GSPELL_REQUIRED_VERSION} required.])
fi

if ! pkg-config --max-version=0.1.99 gspell-1; then
	AC_MSG_ERROR([gspell-1 ${GSPELL_REQUIRED_VERSION} required.])
fi

if ! pkg-config gsettings-desktop-schemas; then
	AC_MSG_ERROR([gsettings-desktop-schemas required.])
fi

if ! pkg-config --atleast-version=${GEE_REQUIRED_VERSION} gee-0.8; then
	AC_MSG_ERROR([gee-0.8 >= ${GEE_REQUIRED_VERSION} required.])
fi

dependencies="
	glib-2.0
	gio-2.0
	gtk+-3.0
	gtksourceview-3.0
	gspell-1
	gsettings-desktop-schemas
	gee-0.8"

CFLAGS="$CFLAGS `pkg-config --cflags ${dependencies}`"
LIBS="$LIBS `pkg-config --libs ${dependencies}`"

# Native Language Support
IT_PROG_INTLTOOL([0.50.1])

GETTEXT_PACKAGE=${PACKAGE_TARNAME}
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],
                   ["${GETTEXT_PACKAGE}"],
                   [The domain to use with gettext])

adl_RECURSIVE_EVAL(["${localedir}"], [LOCALE_DIR])
AC_DEFINE_UNQUOTED([LOCALE_DIR],
                   ["${LOCALE_DIR}"],
		   [locale directory])

# Misc
YELP_HELP_INIT
GLIB_GSETTINGS
GNOME_COMPILE_WARNINGS([maximum])
GOBJECT_INTROSPECTION_CHECK([1.30.0])
VAPIGEN_CHECK
GTK_DOC_CHECK([1.14], [--flavour no-tmpl])
GNOME_CODE_COVERAGE

# Generate files
AC_CONFIG_FILES([Makefile
		 data/Makefile
		 data/build_tools/Makefile
		 data/latexila.desktop.in
		 data/icons/Makefile
		 data/icons/16x16/Makefile
		 data/icons/16x16/apps/Makefile
		 data/icons/22x22/Makefile
		 data/icons/22x22/apps/Makefile
		 data/icons/24x24/Makefile
		 data/icons/24x24/apps/Makefile
		 data/icons/32x32/Makefile
		 data/icons/32x32/apps/Makefile
		 data/icons/48x48/Makefile
		 data/icons/48x48/apps/Makefile
		 data/icons/128x128/Makefile
		 data/icons/128x128/apps/Makefile
		 data/icons/256x256/Makefile
		 data/icons/256x256/apps/Makefile
		 data/icons/scalable/Makefile
		 data/icons/scalable/apps/Makefile
		 data/images/Makefile
		 data/images/app/Makefile
		 data/org.gnome.latexila.gschema.xml
		 data/templates/Makefile
		 docs/Makefile
		 docs/reference/Makefile
		 help/Makefile
		 man/Makefile
		 po/Makefile.in
                 src/Makefile
		 src/evince/Makefile
		 src/gedit/Makefile
		 src/liblatexila/Makefile
		 src/ui/Makefile
		 tests/Makefile
                 README])

AC_OUTPUT

if ${found_vala}
then
	vala_compiler="valac >= ${VALA_REQUIRED_VERSION} not found."
else
	vala_compiler=${VALAC}
fi

# Print a summary of the configuration
echo "

Configuration:

    ${PACKAGE_NAME} version ${PACKAGE_VERSION}

    Prefix: ${prefix}
    Vala Compiler: ${vala_compiler}
    C Compiler: ${CC}
    GObject introspection: ${found_introspection}
    Generate latexila.vapi: ${enable_vala}
"
