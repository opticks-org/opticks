set (HEADER_FILES
    SampleHdf4Importer.h
)
set (SOURCE_FILES
    SampleHdf4Importer.cpp
    ModuleManager.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Hdf4_INCLUDE_DIRS})
include_directories(${Opticks_HdfPlugInLib_INCLUDE_DIR})
add_library(PlugInSamplerHdf SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(PlugInSamplerHdf
    ${Opticks_LIBRARIES}
    ${Opticks_HdfPlugInLib_LIBRARY}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${Hdf4_LIBRARIES}
)
if(WIN32)
    install(TARGETS PlugInSamplerHdf RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS PlugInSamplerHdf RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS PlugInSamplerHdf LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS PlugInSamplerHdf LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
