add_definitions(-DTRANSLATION_DOMAIN=\"libksane\")


include_directories(${SANE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/options
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

set(ksane_SRCS
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
    ksanedevicedialog.cpp
    ksanefinddevicesthread.cpp
    ksanewidget.cpp
    ksanescanthread.cpp
    ksanepreviewthread.cpp
    ksanewidget_p.cpp
    splittercollapser.cpp
    ksaneauth.cpp
    options/ksaneoption.cpp
    options/ksaneoptbutton.cpp
    options/ksaneoptcheckbox.cpp
    options/ksaneoptentry.cpp
    options/ksaneoptgamma.cpp
    options/ksaneoptslider.cpp
    options/ksaneoptfslider.cpp
    options/ksaneoptcombo.cpp
)

add_library(KF5Sane ${ksane_SRCS})
generate_export_header(KF5Sane BASE_NAME KSane)
add_library(KF5::Sane ALIAS KF5Sane)

target_include_directories(KF5Sane INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KSane>")

target_link_libraries(KF5Sane
    PUBLIC
        Qt5::Widgets
    PRIVATE
        ${SANE_LIBRARY}

        KF5::I18n
        KF5::Wallet
        KF5::WidgetsAddons
        KF5::TextWidgets
)

set_target_properties(KF5Sane
  PROPERTIES VERSION ${KSANE_VERSION_STRING}
  SOVERSION ${KSANE_SOVERSION}
  EXPORT_NAME "Sane"
)


ecm_generate_headers(KSane_HEADERS
    HEADER_NAMES
        KSaneWidget
    REQUIRED_HEADERS KSane_HEADERS
)

# Install files

# FIXME KF5: KDE4_INSTALL_ICONS
#KDE4_INSTALL_ICONS(${ICON_INSTALL_DIR})

install(TARGETS KF5Sane
  EXPORT KF5SaneTargets
  ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ksane_export.h
  ${KSane_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KSane
  COMPONENT Devel
)
