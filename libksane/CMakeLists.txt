
INCLUDE_DIRECTORIES(${SANE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/options
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

SET(ksane_LIB_SRCS
    widgets/KSaneGammaDisp.cpp
    widgets/KSaneGamma.cpp
    widgets/KSaneCheckBox.cpp
    widgets/KSaneEntry.cpp
    widgets/KSaneCombo.cpp
    widgets/KSaneSliderF.cpp
    widgets/KSaneSlider.cpp
    widgets/KSaneButton.cpp
    widgets/KSaneOptionWidget.cpp
    KSaneViewer.cpp
    KSaneSelectionItem.cpp
    ksane_device_dialog.cpp
    KSaneFindDevicesThread.cpp
    KSaneScanThread.cpp
    KSanePreviewThread.cpp
    splittercollapser.cpp
    #KSaneWidgetPrivate.cpp
    #KSaneWidget.cpp
    options/KSaneOptButton.cpp
    options/KSaneOptCheckBox.cpp
    options/KSaneOptEntry.cpp
    options/KSaneOptGamma.cpp
    options/KSaneOptSlider.cpp
    options/KSaneOptSliderF.cpp
    options/KSaneOptCombo.cpp
    options/KSaneOptInternal.cpp

    options/KSaneOption.cpp

    KSaneDevice.cpp
    KSaneDevicePrivate.cpp
    KSaneAccessSingleton.cpp
)

KDE4_ADD_LIBRARY(ksane SHARED ${ksane_LIB_SRCS})

TARGET_LINK_LIBRARIES(ksane ${KDE4_KDEUI_LIBS} ${SANE_LIBRARY})

SET_TARGET_PROPERTIES(ksane PROPERTIES VERSION ${KSANE_LIB_SO_VERSION_STRING} SOVERSION ${KSANE_LIB_SO_CUR_VERSION})

OPTION(COMPILE_VIEWER_TESTAPP "Compile a small test application for the viewer")

IF (COMPILE_VIEWER_TESTAPP)
    message("Compile testapp")
    kde4_add_executable(viewer_test KSaneViewer.cpp KSaneViewer_test.cpp KSaneSelectionItem.cpp)
    target_link_libraries(viewer_test ${KDE4_KDEUI_LIBS})
ENDIF()

OPTION(COMPILE_TESTAPP "Compile a small test application for KSaneDevice")

IF (COMPILE_TESTAPP)
    message("Compile KSaneDevice testapp")
    kde4_add_executable(testDevice KSaneDeviceTest.cpp)
    target_link_libraries(testDevice ${KDE4_KDEUI_LIBS} ${SANE_LIBRARY} ksane)
ENDIF()

INSTALL(TARGETS ksane ${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL(FILES ksane.h
              libksane_export.h
              KSaneDevice.h
              options/KSaneOption.h
              ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/libksane COMPONENT Devel)

KDE4_INSTALL_ICONS(${ICON_INSTALL_DIR})


SET(INCLUDE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)
GET_TARGET_PROPERTY(LIB_OUT_NAME ksane LOCATION)
GET_FILENAME_COMPONENT(LIB_OUT_NAME ${LIB_OUT_NAME} NAME)
SET(LIBPATH ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})

CONFIGURE_FILE(
    ../KSaneConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KSaneConfig.cmake @ONLY
)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KSaneConfig.cmake
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/cmake/KSane
)
