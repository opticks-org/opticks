set (HEADER_FILES
    Tutorial1.h
    Tutorial2.h
    Tutorial3.h
    Tutorial4.h
    Tutorial5.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    Tutorial1.cpp
    Tutorial2.cpp
    Tutorial3.cpp
    Tutorial4.cpp
    Tutorial5.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(Tutorial SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Tutorial
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS Tutorial RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Tutorial RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Tutorial LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Tutorial LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
