set (HEADER_FILES
    AutoImporter.h
)
set (SOURCE_FILES
    AutoImporter.cpp
    ModuleManager.cpp
) 
include(${QT_USE_FILE})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS} ${Xerces_INCLUDE_DIRS})
add_library(AutoImporter SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(AutoImporter
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}    
)
if(WIN32)
    install(TARGETS AutoImporter RUNTIME DESTINATIONS PlugIns)
else()
    install(TARGETS AutoImporter LIBRARY DESTINATION PlugIns)
endif()
