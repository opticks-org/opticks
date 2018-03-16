set (HEADER_FILES
    ScriptPlugIn.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    ScriptPlugIn.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(Scripts SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Scripts
    ${Opticks_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS Scripts RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Scripts RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Scripts LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Scripts LIBRARY DESTINATION PlugIns CONFIGURATIONS Debug)
endif()
