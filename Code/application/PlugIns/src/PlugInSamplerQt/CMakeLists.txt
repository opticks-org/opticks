set (HEADER_FILES
    CloseNotificationTest.h
    CustomLayerDrawObject.h
    CustomLayerPlugIn.h
    DataPlotterPlugIn.h
    Demo.h
    DemoAlgorithm.h
    DemoInputs.h
    DesktopAPITestProperties.h
    MouseModePlugIn.h
    MultiLayerMovie.h
    MultiMovie.h
    Node.h
    PlotManager.h
    PlugInTester.h
    RasterTimingTest.h
    SampleGeoref.h
    SampleGeorefGui.h
    TiePointTester.h
    TiePointTesterAlgorithm.h
    TiePointTesterInputs.h
)
set (SOURCE_FILES
    AddPlotDlg.cpp
    AnimationTest.cpp
    AnimationTimingTest.cpp
    CloseNotificationTest.cpp
    CustomLayerDrawObject.cpp
    CustomLayerPlugIn.cpp
    DataPlotterDlg.cpp
    DataPlotterPlugIn.cpp
    Demo.cpp
    DemoAlgorithm.cpp
    DemoGuiImp.cpp
    DesktopAPITest.cpp
    DesktopAPITestGui.cpp
    DesktopAPITestProperties.cpp
    DockWindowWidget.cpp
    LogContextMenuActions.cpp
    MenuAndToolBarTest.cpp
    MenuAndToolBarTestGui.cpp
    Modeless.cpp
    ModuleManager.cpp
    MouseModePlugIn.cpp
    MouseModeTest.cpp
    MouseModeTestGui.cpp
    MultiLayerMovie.cpp
    MultiMovie.cpp
    OptionsSample.cpp
    OptionsSuppressibleMsgDlg.cpp
    PixelAspectRatio.cpp
    PixelAspectRatioGui.cpp
    PlotManager.cpp
    PlotPropertiesDlg.cpp
    PlugInSelectorDlg.cpp
    PlugInTester.cpp
    RasterTimingTest.cpp
    SampleGeoref.cpp
    SampleGeorefGui.cpp
    TiePointTester.cpp
    TiePointTesterAlgorithm.cpp
) 
set (MOC_HEADERS
    AddPlotDlg.h
    AnimationTest.h
    AnimationTimingTest.h
    DataPlotterDlg.h
    DemoGuiImp.h
    DesktopAPITest.h
    DesktopAPITestGui.h
    DockWindowWidget.h
    LogContextMenuActions.h
    MenuAndToolBarTest.h
    MenuAndToolBarTestGui.h
    Modeless.h
    MouseModeTest.h
    MouseModeTestGui.h
    OptionsSample.h
    OptionsSuppressibleMsgDlg.h
    PixelAspectRatio.h
    PixelAspectRatioGui.h
    PlotPropertiesDlg.h
    PlugInSelectorDlg.h
    SampleGeorefGui.h
)
set (UI_FILES
    DynamicColormap.ui 
)
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
qt5_wrap_ui(UI_OUTFILES ${UI_FILES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
source_group(moc FILES ${MOC_OUTFILES})
add_library(PlugInSamplerQt SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES} ${UI_OUTFILES})
    add_custom_command(TARGET PlugInSamplerQt
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${_VCPKG_POWERSHELL_PATH} -noprofile -executionpolicy Bypass -file "${Opticks_SOURCE_DIR}/cmake/copy_dependencies.ps1"
                            -targetBinary $<TARGET_FILE:Opticks>
                            -installedDir "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/bin"
                            -baseTargetBinaryDir "${CMAKE_BINARY_DIR}/bin"
                            -OutVariable out
        )
target_link_libraries(PlugInSamplerQt
    ${Opticks_LIBRARIES}
	${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS PlugInSamplerQt RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS PlugInSamplerQt RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS PlugInSamplerQt LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS PlugInSamplerQt LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
