set (HEADER_FILES
    GetDataSetWavelengths.h
    SetDataSetWavelengths.h
    WavelengthExporter.h
    WavelengthImporter.h
    WavelengthMetadataExporter.h
    WavelengthMetadataImporter.h
    WavelengthTextExporter.h
    WavelengthTextImporter.h
)
set (SOURCE_FILES
    GetDataSetWavelengths.cpp
    ModuleManager.cpp
    SetDataSetWavelengths.cpp
    WavelengthExporter.cpp
    WavelengthImporter.cpp
    WavelengthMetadataExporter.cpp
    WavelengthMetadataImporter.cpp
    WavelengthTextExporter.cpp
    WavelengthTextImporter.cpp
) 
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
include(${QT_USE_FILE})
add_library(Wavelength SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(Wavelength
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS Wavelength RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Wavelength RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Wavelength LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Wavelength LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
