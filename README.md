# KSane

## Introduction

SANE Library interface for KDE

## Authors

See AUTHORS file for details.

## About

Libksane is a KDE interface for SANE library to control flat scanners.

The library documentation is available on header files.

## Dependencies

CMake      >= 2.4.x                     http://www.cmake.org
libqt      >= 4.2.x                     http://www.qtsoftware.com
libkde     >= 4.0.x                     http://www.kde.org
libsane    >= 1.0.18                    http://www.sane-project.org

## Install

In order to compile, especially when QT3/Qt4 are installed at the same time, 
just use something like that:

# export VERBOSE=1
# export QTDIR=/usr/lib/qt4/  
# export PATH=$QTDIR/bin:$PATH 
# cmake .
# make

Usual CMake options:

-DCMAKE_INSTALL_PREFIX : decide where the program will be install on your computer.
-DCMAKE_BUILD_TYPE     : decide which type of build you want. You can chose between "debugfull", "debug", "profile", "relwithdebinfo" and "release". The default is "relwithdebinfo" (-O2 -g).

Compared to old KDE3 autoconf options:

"cmake . -DCMAKE_BUILD_TYPE=debugfull" is equivalent to "./configure --enable-debug=full"
"cmake . -DCMAKE_INSTALL_PREFIX=/usr"  is equivalent to "./configure --prefix=/usr"

More details can be found ata this url: http://techbase.kde.org/Development/Tutorials/CMake#Environment_Variables

Note: To know KDE install path on your computer, use 'kde-config --prefix' command line like this (with full debug object enabled):

"cmake . -DCMAKE_BUILD_TYPE=debugfull -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix`"
