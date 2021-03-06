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
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(CoreIo SHARED ${SOURCE_FILES} ${HEADER_FILES})
link_directories(${Boost_LIBRARY_DIR})
target_link_libraries(CoreIo
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
	Boost::atomic
)
if(WIN32)
    install(TARGETS CoreIo RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS CoreIo RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS CoreIo LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS CoreIo LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
