set (HEADER_FILES
    GcpGeoreference.h
    GeoreferencePlugIn.h
    GeoreferenceUtilities.h
    IgmGeoreference.h
)
set (SOURCE_FILES
    GcpGeoreference.cpp
    GcpGui.cpp
    GeoreferenceDlg.cpp
    GeoreferencePlugIn.cpp
    GeoreferenceUtilities.cpp
    IgmGeoreference.cpp
    IgmGui.cpp
    ModuleManager.cpp
) 
set (MOC_HEADERS
    GcpGui.h
    GeoreferenceDlg.h
    IgmGui.h
)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
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
install(TARGETS Georeference RUNTIME LIBRARY DESTINATION PlugIns)
