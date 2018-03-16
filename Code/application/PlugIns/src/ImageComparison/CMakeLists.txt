set (HEADER_FILES
    ChangeUpDirection.h
)
set (SOURCE_FILES
    ChangeUpDirection.cpp
    ModuleManager.cpp
) 
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(ImageComparison SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(ImageComparison
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS ImageComparison RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS ImageComparison RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS ImageComparison LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS ImageComparison LIBRARY DESTINATION PlugIns CONFIGURATIONS Debug)
endif()
