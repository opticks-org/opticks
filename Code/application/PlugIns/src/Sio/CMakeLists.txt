set (HEADER_FILES
    Sio.h 
    SioImporter.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    Sio.cpp
    SioImporter.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
add_library(Sio SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Sio
    ${Opticks_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS Sio RUNTIME DESTINATION PlugIns)
else()
    install(TARGETS Sio LIBRARY DESTINATION PlugIns)
endif()
