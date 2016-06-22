#!/bin/sh

# not 100% sure
gpg_key=2C472470
gpg2 --export $gpg_key > latexila.gpg

rm -rf latexila/ repo/
flatpak-builder latexila org.gnome.latexila.json || exit 1
flatpak build-export --gpg-sign=$gpg_key repo latexila
