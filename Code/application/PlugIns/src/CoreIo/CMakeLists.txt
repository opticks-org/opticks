set (HEADER_FILES
    CgmExporter.h
    CgmImporter.h
    LayerExporter.h
    LayerImporter.h
    MetadataExporter.h
    ModelExporter.h
    ModelImporter.h
)
set (SOURCE_FILES
    CgmExporter.cpp
    CgmImporter.cpp
    LayerExporter.cpp
    LayerImporter.cpp
    MetadataExporter.cpp
    ModelExporter.cpp
    ModelImporter.cpp
    ModuleManager.cpp
) 
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(CoreIo SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(CoreIo
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
