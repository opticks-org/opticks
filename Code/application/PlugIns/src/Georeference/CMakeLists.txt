set (HEADER_FILES
    GcpGeoreference.h
    GcpGui.h
    GeoreferenceDlg.h
    GeoreferencePlugIn.h
    IgmGeoreference.h
    IgmGui.h
)
set (SOURCE_FILES
    GcpGeoreference.cpp
    GcpGui.cpp
    GeoreferenceDlg.cpp
    GeoreferencePlugIn.cpp
    IgmGeoreference.cpp
    IgmGui.cpp
    ModuleManager.cpp
) 
set (MOC_HEADERS
    GcpGeoreference.h
    GcpGui.h
    GeoreferenceDlg.h
    GeoreferencePlugIn.h
    IgmGeoreference.h
    IgmGui.h
)
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Ossim_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(Georeference SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(Georeference
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Ossim_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS Georeference RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Georeference RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Georeference LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Georeference LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
