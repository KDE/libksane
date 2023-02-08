add_library(KF5Sane)
add_library(KF5::Sane ALIAS KF5Sane)
add_library(KF5::SaneWidget ALIAS KF5Sane)

target_compile_definitions(KF5Sane PRIVATE -DTRANSLATION_DOMAIN=\"libksane\")

if (KF5Wallet_FOUND)
    target_compile_definitions(KF5Sane PRIVATE -DHAVE_KF5WALLET)
    target_link_libraries(KF5Sane PRIVATE KF${KF_MAJOR_VERSION}::Wallet)
endif()

target_sources(KF5Sane PRIVATE
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

ecm_qt_declare_logging_category(KF5Sane
  HEADER ksane_debug.h
  IDENTIFIER KSANE_LOG
  CATEGORY_NAME org.kde.ksane
)

generate_export_header(KF5Sane BASE_NAME KSane)

target_include_directories(KF5Sane
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KSane>"
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

target_link_libraries(KF5Sane
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Widgets
    PRIVATE
        KF${KF_MAJOR_VERSION}::I18n
        KF${KF_MAJOR_VERSION}::WidgetsAddons
        KF${KF_MAJOR_VERSION}::TextWidgets
        KSane::Core
)

set_target_properties(KF5Sane
  PROPERTIES VERSION ${KSANE_VERSION}
  SOVERSION ${KSANE_SOVERSION}
  EXPORT_NAME "Sane"
)

ecm_generate_headers(KSane_HEADERS
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

install(TARGETS KF5Sane
  EXPORT KF5SaneTargets
  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ksane_export.h
  ${KSane_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KSane
  COMPONENT Devel
)
