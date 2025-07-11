cmake_minimum_required(VERSION 3.16.0)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "25")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(KSane VERSION ${RELEASE_SERVICE_VERSION})

message(STATUS "-------------------------------------------")
message(STATUS "Starting CMake configuration for: libksane")

set(QT_REQUIRED_VERSION "6.4.0")
set(KF_MIN_VERSION "6.0.0")

# ECM setup (Extended Cmake Modules)
include(FeatureSummary)
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
# ECM setup2 and include find same cmake module (copied from /usr/share/kde4/apps/cmake/modules/FindSane.cmake)
set(CMAKE_MODULE_PATH ${KSane_SOURCE_DIR}/cmake-modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMInstallIcons)
include(CMakePackageConfigHelpers)
include(CheckFunctionExists)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)

set(KSANEWIDGETS_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR}/KSaneWidgets6")
set(KSANEWIDGETS_CMAKECONFIG_NAME "KSaneWidgets6")
set(LIBRARYFILE_NAME "KSaneWidgets6")
set(TARGET_EXPORT_NAME "KSaneWidgets6")
set(TARGET_EXPORT_NAMESPACE_NAME "")
set(SOVERSION 6)
set(KSANECORE_NAME "KSaneCore6")
set(KSANECORE_TARGET "KSane6::Core")

set(ksane_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/ksane_version.h")
ecm_setup_version(
  ${RELEASE_SERVICE_VERSION}
  VARIABLE_PREFIX KSANE
  VERSION_HEADER "${ksane_version_header}"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${KSANEWIDGETS_CMAKECONFIG_NAME}ConfigVersion.cmake"
  SOVERSION ${SOVERSION}
)
# Dependencies
find_package(Qt6 ${QT_REQUIRED_VERSION} NO_MODULE REQUIRED Core Widgets)

# Required KDE Frameworks
find_package(KF6I18n ${KF_MIN_VERSION} REQUIRED)

if (NOT WIN32)
    find_package(KF6WidgetsAddons ${KF_MIN_VERSION} REQUIRED)
    find_package(KF6TextWidgets ${KF_MIN_VERSION} REQUIRED)

    # Optional KDE Frameworks
    find_package(KF6Wallet ${KF_MIN_VERSION})
    set_package_properties(KF6Wallet PROPERTIES DESCRIPTION "Safe desktop-wide storage for passwords" TYPE OPTIONAL
        PURPOSE "Required to have permanent storage of passwords for scanners"
    )

    # Check if sane API is available.
    find_package(${KSANECORE_NAME} "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}" REQUIRED)
    add_subdirectory(src)
    if (BUILD_TESTING)
        add_subdirectory(autotests)
        add_subdirectory(tests)
    endif()
else ()
    add_subdirectory(ksanetwain)
endif()

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
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F02
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055800
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/${KSANEWIDGETS_CMAKECONFIG_NAME}")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KFSaneConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${KSANEWIDGETS_CMAKECONFIG_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${KSANEWIDGETS_CMAKECONFIG_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${KSANEWIDGETS_CMAKECONFIG_NAME}ConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KSaneWidgetsTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE ${KSANEWIDGETS_CMAKECONFIG_NAME}Targets.cmake
  NAMESPACE ${TARGET_EXPORT_NAMESPACE_NAME}
)

install(FILES
  "${ksane_version_header}"
  DESTINATION ${KSANEWIDGETS_INSTALL_INCLUDEDIR}
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
