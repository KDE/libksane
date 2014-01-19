include_directories(${SANE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/options
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

set(ksane_LIB_SRCS
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
    ksane_widget.cpp
    ksane_scan_thread.cpp
    ksane_preview_thread.cpp
    ksane_widget_private.cpp
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

add_library(ksane SHARED ${ksane_LIB_SRCS})

target_link_libraries (ksane
    LINK_PRIVATE
      KF5::I18n
      KF5::PrintUtils
      KF5::Wallet
      KF5::WidgetsAddons
      ${SANE_LIBRARY})

set_target_properties(ksane PROPERTIES VERSION ${KSANE_LIB_SO_VERSION_STRING} SOVERSION ${KSANE_LIB_SO_CUR_VERSION})

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

install(TARGETS ksane ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES ksane.h
              libksane_export.h
              ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libksane COMPONENT Devel)

# TODO: KDE4_INSTALL_ICONS
#KDE4_INSTALL_ICONS(${ICON_INSTALL_DIR})


configure_file(
    ../KSaneConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KSaneConfig.cmake @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KSaneConfig.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/KSane
)
