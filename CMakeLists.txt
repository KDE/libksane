PROJECT(libksane)

# =======================================================
# Informations to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0

# Library API version
SET(KSANE_LIB_MAJOR_VERSION "0")
SET(KSANE_LIB_MINOR_VERSION "1")
SET(KSANE_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-svn"   : alpha code unstable from svn. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(KSANE_LIB_SUFFIX_VERSION "-svn")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual.html#Updating-version-info
SET(KSANE_LIB_SO_CUR_VERSION "0")
SET(KSANE_LIB_SO_REV_VERSION "1")
SET(KSANE_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

SET(KSANE_LIB_VERSION_STRING "${KSANE_LIB_MAJOR_VERSION}.${KSANE_LIB_MINOR_VERSION}.${KSANE_LIB_PATCH_VERSION}${KSANE_LIB_SUFFIX_VERSION}")
SET(KSANE_LIB_VERSION_ID "0x0${KSANE_LIB_MAJOR_VERSION}0${KSANE_LIB_MINOR_VERSION}0${KSANE_LIB_PATCH_VERSION}")

SET(KSANE_LIB_SO_VERSION_STRING "${KSANE_LIB_SO_CUR_VERSION}.${KSANE_LIB_SO_REV_VERSION}.${KSANE_LIB_SO_AGE_VERSION}")

# =======================================================


SUBDIRS(libksane)

# Detect Exiv2 library properlly with CMake script from 'cmake/modules'
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )
FIND_PACKAGE(Exiv2 REQUIRED)

IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libksane.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libksane.pc)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libksane.lsm.cmake ${CMAKE_CURRENT_BINARY_DIR}/libksane.lsm)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libksane.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDIF(NOT WIN32)
