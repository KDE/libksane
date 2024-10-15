add_library(KSaneWidgets)

set_target_properties(KSaneWidgets PROPERTIES
    VERSION     ${KSANE_VERSION}
    SOVERSION   ${KSANE_SOVERSION}
    EXPORT_NAME ${TARGET_EXPORT_NAME}
    OUTPUT_NAME ${LIBRARYFILE_NAME}
)

target_compile_definitions(KSaneWidgets PRIVATE -DTRANSLATION_DOMAIN=\"libksane\")

IF( WIN32 )
    target_compile_definitions(KSaneWidgets PRIVATE -DMAKE_KSANE_LIB )
ENDIF( WIN32 )

target_sources(KSaneWidgets PRIVATE
    twainiface.cpp
    ktwain_widget.cpp
    ../src/ksanewidget.h
)

generate_export_header(KSaneWidgets BASE_NAME KSane)

target_include_directories(KSaneWidgets
    INTERFACE
        "$<INSTALL_INTERFACE:${KSANEWIDGETS_INSTALL_INCLUDEDIR}>"
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(KSaneWidgets
    PUBLIC
        Qt6::Widgets
    PRIVATE
        KF6::I18n
)

ecm_generate_headers(KSane_HEADERS
    HEADER_NAMES
        KSaneWidget
    RELATIVE "../src/"
    REQUIRED_HEADERS KSane_HEADERS
)

install(TARGETS KSaneWidgets
    EXPORT KSaneWidgetsTargets
    ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ksane_export.h
    ${KSane_HEADERS}
    DESTINATION ${KSANEWIDGETS_INSTALL_INCLUDEDIR}
    COMPONENT Devel
)
