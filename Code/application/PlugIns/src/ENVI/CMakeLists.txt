set (HEADER_FILES
    EnviExporter.h
    EnviField.h
    EnviImporter.h
    EnviLibraryExporter.h
    EnviLibraryImporter.h
)
set (SOURCE_FILES
    EnviExporter.cpp
    EnviField.cpp
    EnviImporter.cpp
    EnviLibraryExporter.cpp
    EnviLibraryImporter.cpp
    ModuleManager.cpp
) 
set (MOC_HEADERS
    AspamPlotSelectionDialog.h
    AspamViewerDialog.h
)
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(ENVI SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(ENVI
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
install(TARGETS ENVI RUNTIME DESTINATION PlugIns)