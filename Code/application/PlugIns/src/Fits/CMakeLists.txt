set (HEADER_FILES
    FitsImporter.h
)
set (SOURCE_FILES
    FitsImporter.cpp
    ModuleManager.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${cfitsio_INCLUDE_DIRS})
add_library(Fits SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Fits
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}
    ${cfitsio_LIBRARIES}
    ${Xerces_LIBRARIES}
    ${PTHREADS_LIBRARY}
)
if(WIN32)
    install(TARGETS Fits RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Fits RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Fits LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Fits LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
