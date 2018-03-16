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
install(TARGETS Scripts RUNTIME LIBRARY DESTINATION PlugIns)
