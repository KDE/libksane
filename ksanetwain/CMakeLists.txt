add_library(KF5Sane)
add_library(KF5::Sane ALIAS KF5Sane)

target_compile_definitions(KF5Sane PRIVATE -DTRANSLATION_DOMAIN=\"libksane\")

IF( WIN32 )
    target_compile_definitions(KF5Sane PRIVATE -DMAKE_KSANE_LIB )
ENDIF( WIN32 )

target_sources(KF5Sane PRIVATE
    twainiface.cpp
    ktwain_widget.cpp
    ../src/ksanewidget.h
)

generate_export_header(KF5Sane BASE_NAME KSane)

target_include_directories(KF5Sane
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KSane>"
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(KF5Sane
    PUBLIC
        Qt5::Widgets
    PRIVATE
        KF5::I18n
)

set_target_properties(KF5Sane
    PROPERTIES VERSION ${KSANE_VERSION}
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
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ksane_export.h
    ${KSane_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KSane
    COMPONENT Devel
)
