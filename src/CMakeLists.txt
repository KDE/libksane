include_directories(${SANE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/options
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

set(ksane_SRCS
    widgets/gamma_disp.cpp
    widgets/labeled_gamma.cpp
    widgets/labeled_checkbox.cpp
    widgets/labeled_entry.cpp
    widgets/labeled_combo.cpp
    widgets/labeled_fslider.cpp
    widgets/labeled_slider.cpp
    widgets/ksane_button.cpp
    widgets/ksane_option_widget.cpp
    ksane_viewer.cpp
    selectionitem.cpp
    ksane_device_dialog.cpp
    ksane_find_devices_thread.cpp
    ksane.cpp
    ksane_scan_thread.cpp
    ksane_preview_thread.cpp
    ksane_p.cpp
    splittercollapser.cpp
    ksane_auth.cpp
    options/ksane_option.cpp
    options/ksane_opt_button.cpp
    options/ksane_opt_checkbox.cpp
    options/ksane_opt_entry.cpp
    options/ksane_opt_gamma.cpp
    options/ksane_opt_slider.cpp
    options/ksane_opt_fslider.cpp
    options/ksane_opt_combo.cpp
)

add_library(KF5Sane ${ksane_SRCS})
generate_export_header(KF5Sane BASE_NAME KSane)
add_library(KF5::KSane ALIAS KF5Sane)

target_include_directories(KF5Sane INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KSane>")

target_link_libraries(KF5Sane
  PUBLIC
    Qt5::Core
  # PRIVATE
  #  KF5::CoreAddons
  #  Other private libs
    KF5::I18n
    KF5::PrintUtils
    KF5::Wallet
    KF5::WidgetsAddons
    ${SANE_LIBRARY}
)

set_target_properties(KF5Sane
  PROPERTIES VERSION ${KSANE_VERSION_STRING}
  SOVERSION ${KSANE_SOVERSION}
)


option(COMPILE_VIEWER_TESTAPP "Compile a small test application for the viewer")
if (COMPILE_VIEWER_TESTAPP)
    message("Compile testapp")
    add_executable(viewer_test ksane_viewer.cpp ksane_viewer_test.cpp selectionitem.cpp)
    target_link_libraries(viewer_test
      LINK_PRIVATE
    KF5::I18n
    KF5::PrintUtils
    KF5::Wallet
    KF5::WidgetsAddons
    ${SANE_LIBRARY})
endif()

# FIXME?
# Crazy method to convert wrongly named _ classes to proper KF5 headers
ecm_generate_headers(
  KSane_Viewer
  KSane
  # FIXME? more?

  MODULE_NAME KSane
  REQUIRED_HEADERS KSane_HEADERS
)

# Install files
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/KSane DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

# FIXME KF5: KDE4_INSTALL_ICONS
#KDE4_INSTALL_ICONS(${ICON_INSTALL_DIR})

install(TARGETS KF5Sane
  EXPORT KF5SaneTargets
  ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ksane_export.h
  ${KSane_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/KSane
  COMPONENT Devel
)
