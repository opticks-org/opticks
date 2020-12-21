set (HEADER_FILES
    Kml.h
    KmlExporter.h
)
set (SOURCE_FILES
    Kml.cpp
    KmlExporter.cpp
    ModuleManager.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${Minizip_INCLUDE_DIRS})
include_directories(${Opticks_ServerUtilities_INCLUDE_DIR})
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
source_group(moc FILES ${MOC_OUTFILES})
add_library(Kml SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(Kml
    ${Opticks_LIBRARIES}
    ${Opticks_ServerUtilities_LIBRARY}
    ${Minizip_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
if(WIN32)
    install(TARGETS Kml RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Kml RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Kml LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Kml LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
