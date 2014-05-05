#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
[ -z "$srcdir" ] && srcdir=.

PKG_NAME=latexila
REQUIRED_AUTOMAKE_VERSION=1.7

if [ ! -f "$srcdir/configure.ac" ]; then
 echo "$srcdir doesn't look like source directory for $PKG_NAME" >&2
 exit 1
fi

which gnome-autogen.sh || {
        echo "You need to install gnome-common from GNOME Git"
        exit 1
}

. gnome-autogen.sh "$@"
