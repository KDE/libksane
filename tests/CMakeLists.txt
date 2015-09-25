
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
            KF5Sane
            KF5::I18n
            KF5::Wallet
            KF5::WidgetsAddons
    )
endif()
