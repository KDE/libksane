
option(COMPILE_VIEWER_TESTAPP "Compile a small test application for the viewer")
if (COMPILE_VIEWER_TESTAPP)
    message("Compile testapp")
    add_executable(viewertest
        ${CMAKE_SOURCE_DIR}/src/selectionitem.cpp
        ${CMAKE_SOURCE_DIR}/src/ksaneviewer.cpp
        ksaneviewertest.cpp
    )
    target_link_libraries(viewertest
        PRIVATE
            KSaneWidget
            KF${KF_MAJOR_VERSION}::I18n
            KF${KF_MAJOR_VERSION}::Wallet
            KF${KF_MAJOR_VERSION}::WidgetsAddons
    )
endif()
