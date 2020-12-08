set (HEADER_FILES
    Aspam.h
    AspamAdapter.h
    AspamImp.h
    AspamImporter.h
    AspamManager.h
    AspamViewer.h
)
set (SOURCE_FILES
    AspamImp.cpp
    AspamImporter.cpp
    AspamManager.cpp
    AspamPlotSelectionDialog.cpp
    AspamViewer.cpp
    AspamViewerDialog.cpp
    ModuleManager.cpp
) 
set (MOC_HEADERS
    AspamPlotSelectionDialog.h
    AspamViewerDialog.h
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(Aspam SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(Aspam
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS Aspam RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Aspam RUNTIME DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
else()
    install(TARGETS Aspam LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Aspam LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
