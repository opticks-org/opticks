set (HEADER_FILES
    CreateAnimation.h
    CreateExportFileDescriptor.h
    CreateFileDescriptor.h
    DeriveLayer.h
    DeriveProduct.h
    DesktopItems.h
    EditDataDescriptor.h
    ExportDataSet.h
    GetDataDescriptor.h
    GetFilename.h
    GetPrimaryRasterLayer.h
    GetSessionItem.h
    GetView.h
    ImportDataSet.h
    LayerItems.h
    LoadLayer.h
    ModelItems.h
    PrintView.h
    RunInterpreterCommands.h
    SaveLayer.h
    SelectPlugIn.h
    SetDisplayedBand.h
    SetDisplayMode.h
    SetThresholdOptions.h
    SetViewDisplayArea.h
    TemplateUtilities.h
    ThresholdData.h
    UtilityItems.h
    WizardItems.h
)
set (SOURCE_FILES
    CreateAnimation.cpp
    CreateExportFileDescriptor.cpp
    CreateFileDescriptor.cpp
    DeriveLayer.cpp
    DeriveProduct.cpp
    DesktopItems.cpp
    EditDataDescriptor.cpp
    ExportDataSet.cpp
    GetDataDescriptor.cpp
    GetFilename.cpp
    GetPrimaryRasterLayer.cpp
    GetSessionItem.cpp
    GetView.cpp
    ImportDataSet.cpp
    LayerItems.cpp
    LoadLayer.cpp
    ModelItems.cpp
    ModuleManager.cpp
    PrintView.cpp
    RunInterpreterCommands.cpp
    SaveLayer.cpp
    SelectPlugIn.cpp
    SetDisplayedBand.cpp
    SetDisplayMode.cpp
    SetThresholdOptions.cpp
    SetViewDisplayArea.cpp
    TemplateUtilities.cpp
    ThresholdData.cpp
    UtilityItems.cpp
    WizardItems.cpp
    ZoomAndPanToPoint.cpp
) 
set (MOC_HEADERS
    ZoomAndPanToPoint.h
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(WizardItems SHARED ${SOURCE_FILES} ${HEADER_FILES} ${MOC_HEADERS} ${MOC_OUTFILES})
target_link_libraries(WizardItems
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS WizardItems RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS WizardItems RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS WizardItems LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS WizardItems LIBRARY DESTINATION PlugIns CONFIGURATIONS Debug)
endif()
