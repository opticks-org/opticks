set (HEADER_FILES
    SampleHdf4Importer.h
)
set (SOURCE_FILES
    SampleHdf4Importer.cpp
    ModuleManager.cpp
) 
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Hdf4_INCLUDE_DIRS})
include_directories(${Opticks_HdfPlugInLib_INCLUDE_DIR})
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    include_directories(${JPEG_INCLUDE_DIRS})
endif()
add_library(PlugInSamplerHdf SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(PlugInSamplerHdf
    ${Opticks_LIBRARIES}
    ${Opticks_HdfPlugInLib_LIBRARY}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${Hdf4_LIBRARIES}
)
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    target_link_libraries(PlugInSamplerHdf
        ${szip_LIBRARIES}
        ${Hdf4_LIBRARIES}
        ${JPEG_LIBRARIES}
    )
endif()
