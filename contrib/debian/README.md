
Debian
====================
This directory contains files used to package timed/time-qt
for Debian-based Linux systems. If you compile timed/time-qt yourself, there are some useful files here.

## time: URI support ##


time-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install time-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your time-qt binary to `/usr/bin`
and the `../../share/pixmaps/time128.png` to `/usr/share/pixmaps`

time-qt.protocol (KDE)

