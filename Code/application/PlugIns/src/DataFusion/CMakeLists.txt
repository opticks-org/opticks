set (HEADER_FILES
    DataFusion.h
    DataFusionTests.h
    DataFusionTools.h
    FlickerControls.h
    FusionException.h
    Matrix.h
    Poly2D.h
    Polywarp.h
    Vector.h
)
set (SOURCE_FILES
    DataFusion.cpp
    DataFusionDlg.cpp
    DataFusionTests.cpp
    DataFusionTools.cpp
    DatasetPage.cpp
    FlickerControls.cpp
    FusionAlgorithmInputsPage.cpp
    FusionLayersSelectPage.cpp
    FusionPage.cpp
    ImageAdjustWidget.cpp
    ModuleManager.cpp
    TiePointPage.cpp
) 
set (MOC_HEADERS
    DataFusionDlg.h
    DatasetPage.h
    FusionAlgorithmInputsPage.h
    FusionLayersSelectPage.h
    FusionPage.h
    ImageAdjustWidget.h
    TiePointPage.h
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(DataFusion SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(DataFusion
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS DataFusion RUNTIME DESTINATIONS PlugIns)
else()
    install(TARGETS DataFusion LIBRARY DESTINATION PlugIns)
endif()
