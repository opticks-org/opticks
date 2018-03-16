set (HEADER_FILES
    Passthrough.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    Passthrough.cpp
) 
add_definitions("-DDEPRECATED_MODULE_TYPE" -DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS} ${Xerces_INCLUDE_DIRS})
if(WIN32)
    set(DEF_FILE "${Opticks_SOURCE_DIR}/PlugInManager/PlugInModule.def")
endif()
add_library(LegacyModule SHARED ${SOURCE_FILES} ${HEADER_FILES} ${DEF_FILE})
target_link_libraries(LegacyModule
    ${Opticks_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS LegacyModule RUNTIME DESTINATIONS PlugIns)
else()
    install(TARGETS LegacyModule LIBRARY DESTINATION PlugIns)
endif()
