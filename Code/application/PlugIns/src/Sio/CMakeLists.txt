set (HEADER_FILES
    Sio.h 
    SioImporter.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    Sio.cpp
    SioImporter.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
add_library(Sio SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Sio
    ${Opticks_LIBRARIES}
	${QT_LIBRARIES}
    ${Xerces_LIBRARIES}
    Eigen3::Eigen
)
if(WIN32)
    install(TARGETS Sio RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Sio RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Sio LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Sio LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
