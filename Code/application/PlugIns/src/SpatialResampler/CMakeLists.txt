set (HEADER_FILES
    SpatialResampler.h
    SpatialResamplerOptions.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    SpatialResampler.cpp
    SpatialResamplerOptions.cpp
) 
set (MOC_HEADERS
    SpatialResamplerOptions.h
)
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
source_group(moc FILES ${MOC_OUTFILES})
add_library(SpatialResampler SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(SpatialResampler
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS SpatialResampler RUNTIME DESTINATION PlugIns)
else()
    install(TARGETS SpatialResampler LIBRARY DESTINATION PlugIns)
endif()
