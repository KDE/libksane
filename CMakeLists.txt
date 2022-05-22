cmake_minimum_required(VERSION 3.16.0)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "22")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(KSane VERSION ${RELEASE_SERVICE_VERSION})

message(STATUS "-------------------------------------------")
message(STATUS "Starting CMake configuration for: libksane")

# ECM setup (Extended Cmake Modules)
include(FeatureSummary)
set(KF5_VERSION "5.90.0")
find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
# ECM setup2 and include find same cmake module (copied from /usr/share/kde4/apps/cmake/modules/FindSane.cmake)
set(CMAKE_MODULE_PATH ${KSane_SOURCE_DIR}/cmake-modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMInstallIcons)
include(CMakePackageConfigHelpers)
include(CheckFunctionExists)
include(GenerateExportHeader)
include(ECMQtDeclareLoggingCategory)

ecm_setup_version(
  ${RELEASE_SERVICE_VERSION}
  VARIABLE_PREFIX KSANE
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksane_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SaneConfigVersion.cmake"
  SOVERSION 5
)

# Dependencies
set(REQUIRED_QT_VERSION "5.15.0")
# Required Qt5 components to build this framework
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core Widgets)

# Required KF5 frameworks
find_package(KF5I18n ${KF5_VERSION} REQUIRED)

if (NOT WIN32)
    find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)

    # Optional KF5 frameworks
    find_package(KF5Wallet ${KF5_VERSION})
    set_package_properties(KF5Wallet PROPERTIES DESCRIPTION "Safe desktop-wide storage for passwords" TYPE OPTIONAL
        PURPOSE "Required to have permanent storage of passwords for scanners"
    )

    # Check if sane API is available.
    find_package(Sane REQUIRED)
    message(STATUS "SANE_FOUND:       ${SANE_FOUND}")
    message(STATUS "SANE_INCLUDE_DIR: ${SANE_INCLUDE_DIR}")
    message(STATUS "SANE_LIBRARY:     ${SANE_LIBRARY}")
    # macro_log_feature(SANE_FOUND "SANE development toolkit" "Scanner Access Now Easy (SANE) development package" "http://www.sane-project.org" TRUE "" "Needed by libksane")
endif (NOT WIN32)

add_definitions(
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_KEYWORDS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DQT_STRICT_ITERATORS
)
add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055800
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

# Subdirectories
if (SANE_FOUND)
    add_subdirectory(src)
    if (BUILD_TESTING)
        add_subdirectory(autotests)
        add_subdirectory(tests)
    endif()
else ()
    add_subdirectory(ksanetwain)
endif()

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Sane")

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
  DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}"
  COMPONENT Devel
)

ki18n_install(po)

# =======================================================

# To prevent warnings from M$ compiler
IF(WIN32 AND MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_ATL_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_AFX_SECURE_NO_WARNINGS)
ENDIF(WIN32 AND MSVC)

# =======================================================

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
