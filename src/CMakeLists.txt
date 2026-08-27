add_library(KSaneWidgets)

set_target_properties(KSaneWidgets PROPERTIES
    VERSION     ${KSANE_VERSION}
    SOVERSION   ${KSANE_SOVERSION}
    EXPORT_NAME "KSaneWidgets6"
    OUTPUT_NAME "KSaneWidgets6"
)

target_compile_definitions(KSaneWidgets PRIVATE -DTRANSLATION_DOMAIN=\"libksane\")

if (KF6Wallet_FOUND)
    target_compile_definitions(KSaneWidgets PRIVATE -DHAVE_KWALLET)
    target_link_libraries(KSaneWidgets PRIVATE KF6::Wallet)
endif()

target_sources(KSaneWidgets PRIVATE
    widgets/gammadisp.cpp
    widgets/labeledgamma.cpp
    widgets/labeledcheckbox.cpp
    widgets/labeledentry.cpp
    widgets/labeledcombo.cpp
    widgets/labeledfslider.cpp
    widgets/labeledslider.cpp
    widgets/ksanebutton.cpp
    widgets/ksaneoptionwidget.cpp
    ksaneviewer.cpp
    selectionitem.cpp
    hiderectitem.cpp
    ksanedevicedialog.cpp
    ksanewidget.cpp
    ksanewidget_p.cpp
    splittercollapser.cpp
)

ecm_qt_declare_logging_category(KSaneWidgets
  HEADER ksane_debug.h
  IDENTIFIER KSANE_LOG
  CATEGORY_NAME org.kde.ksane
)

ecm_generate_export_header(KSaneWidgets
    BASE_NAME KSane
    VERSION ${KSANE_VERSION}
    DEPRECATED_BASE_VERSION 0
    USE_VERSION_HEADER
)

target_include_directories(KSaneWidgets
    INTERFACE
        "$<INSTALL_INTERFACE:${KSANEWIDGETS_INSTALL_INCLUDEDIR}>"
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

target_link_libraries(KSaneWidgets
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    PRIVATE
        KF6::I18n
        KF6::WidgetsAddons
        KF6::TextWidgets
        KSane6::Core
)

ecm_generate_headers(KSane_CamelCase_HEADERS
    HEADER_NAMES
        KSaneWidget
    REQUIRED_HEADERS KSane_HEADERS
    RELATIVE "../src/"
)

# Install files

ecm_install_icons(ICONS
  16-actions-black-white.png
  16-actions-color.png
  16-actions-gray-scale.png
  DESTINATION ${KDE_INSTALL_ICONDIR})

install(TARGETS KSaneWidgets
  EXPORT KSaneWidgetsTargets
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ksane_export.h
  ${KSane_HEADERS}
  ${KSane_CamelCase_HEADERS}
  DESTINATION ${KSANEWIDGETS_INSTALL_INCLUDEDIR}
  COMPONENT Devel
)

if(${ECM_VERSION} VERSION_GREATER_EQUAL 6.30)
include(ECMInstalledLibraryCheck)

ecm_add_installed_library_check(KSaneWidgets
    PACKAGE_NAME "KSaneWidgets6"
    NO_PACKAGE_TARGET_NAMESPACE
)

ecm_installed_library_check_include_strings(KSaneWidgets
    HEADERS ${KSane_CamelCase_HEADERS}
)

ecm_installed_library_check_version_preprocessor_macros(KSaneWidgets
    PREFIX KSANE
)
endif()
