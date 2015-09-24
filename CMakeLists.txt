cmake_minimum_required(VERSION 2.8.12)

project(KSane)

message(STATUS "-------------------------------------------")
message(STATUS "Starting CMake configuration for: libksane")

# ECM setup (Extended Cmake Modules)
find_package(ECM 1.1.0 REQUIRED NO_MODULE)
# ECM setup2 and include find same cmake module (copied from /usr/share/kde4/apps/cmake/modules/FindSane.cmake)
set(CMAKE_MODULE_PATH ${KSane_SOURCE_DIR}/cmake-modules ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(CheckFunctionExists)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
include(GenerateExportHeader)

set(KF5_VERSION "5.1.0")

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KSANE
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksane_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SaneConfigVersion.cmake"
)

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

# Required Qt5 components to build this framework
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets)

# Required KF5 frameworks
find_package(KF5Config ${KF5_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_VERSION} REQUIRED)
find_package(KF5Wallet ${KF5_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
# ...

# be not so strict with the poor developer who just wants to use strings
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_URL_CAST_FROM_STRING)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

# Check if sane API is available.
if (NOT WIN32)
    find_package(Sane)
    message(STATUS "SANE_FOUND:       ${SANE_FOUND}")
    message(STATUS "SANE_INCLUDE_DIR: ${SANE_INCLUDE_DIR}")
    message(STATUS "SANE_LIBRARY:     ${SANE_LIBRARY}")
    # macro_log_feature(SANE_FOUND "SANE development toolkit" "Scanner Access Now Easy (SANE) development package" "http://www.sane-project.org" TRUE "" "Needed by libksane")
endif (NOT WIN32)

# Subdirectories
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Sane")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5SaneConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SaneConfig.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SaneConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SaneConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KF5SaneTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KF5SaneTargets.cmake
  NAMESPACE KF5::
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ksane_version.h"
  DESTINATION "${KF5_INCLUDE_INSTALL_DIR}"
  COMPONENT Devel
)

# needed?
#include_directories (${CMAKE_SOURCE_DIR}
#                     ${CMAKE_BINARY_DIR}
#                     ${CMAKE_CURRENT_BINARY_DIR}/libksane)

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0
# 0.1.1 => 0.1.1
# 0.2.0 => 0.2.0

# Library API version
SET(KSANE_LIB_MAJOR_VERSION "0")
SET(KSANE_LIB_MINOR_VERSION "3")
SET(KSANE_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(KSANE_LIB_SUFFIX_VERSION "")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
SET(KSANE_LIB_SO_CUR_VERSION "0")
SET(KSANE_LIB_SO_REV_VERSION "2")
SET(KSANE_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

SET(KSANE_LIB_VERSION_STRING "${KSANE_LIB_MAJOR_VERSION}.${KSANE_LIB_MINOR_VERSION}.${KSANE_LIB_PATCH_VERSION}${KSANE_LIB_SUFFIX_VERSION}")
SET(KSANE_LIB_VERSION_ID "0x0${KSANE_LIB_MAJOR_VERSION}0${KSANE_LIB_MINOR_VERSION}0${KSANE_LIB_PATCH_VERSION}")

SET(KSANE_LIB_SO_VERSION_STRING "${KSANE_LIB_SO_CUR_VERSION}.${KSANE_LIB_SO_REV_VERSION}.${KSANE_LIB_SO_AGE_VERSION}")

# =======================================================

ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=51004) 

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/version.h)

# =======================================================

# To prevent warnings from M$ compiler
IF(WIN32 AND MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_ATL_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_AFX_SECURE_NO_WARNINGS)
ENDIF(WIN32 AND MSVC)

# =======================================================

#IF (SANE_FOUND)
#    # ADD_SUBDIRECTORY(src)  # old: was already done above
#ELSE (SANE_FOUND)
#    ADD_SUBDIRECTORY(ksanetwain)
#.ENDIF (SANE_FOUND)

IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libksane.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libksane.pc)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libksane.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDIF(NOT WIN32)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

