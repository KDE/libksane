INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}
)

IF( WIN32 )
    ADD_DEFINITIONS( -DMAKE_KSANE_LIB )
ENDIF( WIN32 )

set(ktwain_src
    twainiface.cpp
    ktwain_widget.cpp
    ../src/ksanewidget.h
)

add_library(KF5Sane ${ktwain_src})
generate_export_header(KF5Sane BASE_NAME KSane)
add_library(KF5::Sane ALIAS KF5Sane)

target_include_directories(KF5Sane INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KSane>")

target_link_libraries(KF5Sane
    PUBLIC
        Qt5::Widgets
    PRIVATE
        KF5::I18n
)

set_target_properties(KF5Sane
    PROPERTIES VERSION ${KSANE_VERSION_STRING}
    SOVERSION ${KSANE_SOVERSION}
    EXPORT_NAME "Sane"
)


ecm_generate_headers(KSane_HEADERS
    HEADER_NAMES
        KSaneWidget
    RELATIVE "../src/"
    REQUIRED_HEADERS KSane_HEADERS
)

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
