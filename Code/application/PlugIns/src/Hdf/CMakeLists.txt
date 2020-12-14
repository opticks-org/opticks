set (HEADER_FILES
    GenericHdf5Importer.h
    HyperionImporter.h
)
set (SOURCE_FILES
    GenericHdf5Importer.cpp
    HyperionImporter.cpp
    ModuleManager.cpp
) 
add_definitions(${Hdf5_DEFINITIONS})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Hdf5_INCLUDE_DIRS})
include_directories(${Hdf4_INCLUDE_DIRS})
include_directories(${Opticks_HdfPlugInLib_INCLUDE_DIR})
add_library(Hdf SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Hdf
    ${Opticks_LIBRARIES}
    ${Opticks_HdfPlugInLib_LIBRARY}
    ${Xerces_LIBRARIES}
    ${Hdf4_LIBRARIES}
    ${PTHREADS_LIBRARY}
    ${Hdf5_LIBRARIES}
)
if(WIN32)
    install(TARGETS Hdf RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Hdf RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Hdf LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Hdf LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
