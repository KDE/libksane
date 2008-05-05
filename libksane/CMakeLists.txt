INCLUDE_DIRECTORIES(${KDE4_INCLUDES} ${QT4_INCLUDES})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

SET(ksane_LIB_SRCS gamma_disp.cpp
                   labeled_gamma.cpp
                   labeled_checkbox.cpp
                   labeled_entry.cpp
                   labeled_separator.cpp
                   preview_image.cpp
                   sane_option.cpp
                   labeled_combo.cpp
                   labeled_fslider.cpp
                   labeled_slider.cpp
                   preview_area.cpp
                   radio_select.cpp
                   ksane_button.cpp
                   sane_widget.cpp)

KDE4_ADD_LIBRARY(ksane SHARED ${ksane_LIB_SRCS})

TARGET_LINK_LIBRARIES(ksane ${KDE4_KDEUI_LIBS}
                            ${SANE_LIBRARY}
                     )

SET_TARGET_PROPERTIES(ksane PROPERTIES VERSION ${KSANE_LIB_SO_VERSION_STRING} SOVERSION ${KSANE_LIB_SO_CUR_VERSION})

INSTALL(TARGETS ksane DESTINATION ${LIB_INSTALL_DIR})

INSTALL(FILES ksane.h
              libksane_export.h
              ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libksane)

KDE4_INSTALL_ICONS(${ICON_INSTALL_DIR})
