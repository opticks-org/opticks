set (HEADER_FILES
    GenericImporter.h
)
set (SOURCE_FILES
    GenericImporter.cpp
    ModuleManager.cpp
) 
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(Generic SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Generic
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
install(TARGETS Generic RUNTIME LIBRARY DESTINATION PlugIns)
