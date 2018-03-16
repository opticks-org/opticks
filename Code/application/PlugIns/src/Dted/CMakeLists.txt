set (HEADER_FILES
    AccHeader.h
    DsiHeader.h
    DtedImporter.h
    DtedShared.h
    UhlHeader.h
)
set (SOURCE_FILES
    AccHeader.cpp
    DsiHeader.cpp
    DtedImporter.cpp
    DtedShared.cpp
    UhlHeader.cpp
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
add_library(Dted SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Dted
    ${Opticks_LIBRARIES}
    ${Xerces_LIBRARIES}
)
install(TARGETS Dted RUNTIME DESTINATION PlugIns)