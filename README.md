# KSane

## Introduction

SANE Library Qt-based interface

## Authors

See AUTHORS file for details.

## About

Libksane is a Qt-based interface for SANE library to control flat scanners.

The library documentation is available on header files.

## Dependencies

 - CMake          >= 3.11.0                https://cmake.org
 - Qt             >= 5.14.0                https://qt.io
 - KDE Frameworks >= 5.80.0                https://www.kde.org
 - libsane        >= 1.0.18                http://www.sane-project.org

## Install

In order to compile just use something like that:

```bash
export VERBOSE=1
export QTDIR=/usr/lib/qt5/
export PATH=$QTDIR/bin:$PATH 
cmake .
make
```

Usual CMake options:

| Option                | Description |
| -------               | ----------- |
| DCMAKE_INSTALL_PREFIX | decide where the program will be install on your computer.le |
| DCMAKE_BUILD_TYPE     | decide which type of build you want. You can chose between Debug, Release, RelWithDebInfo and MinSizeRel. The default is RelWithDebInfo (-O2 -g). |

More details can be found at this url: https://community.kde.org/Guidelines_and_HOWTOs/CMake

Note: To know the KDE Frameworks install path on your computer, use `kf5-config --prefix` command line like this (with full debug object enabled):

```bash
cmake . -DCMAKE_BUILD_TYPE=debugfull -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix`
```
