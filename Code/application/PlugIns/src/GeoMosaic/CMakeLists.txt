set (HEADER_FILES
    GeoMosaic.h
    GeoMosaicChip.h
    MosaicManager.h
)
set (SOURCE_FILES
    GeoMosaic.cpp
    GeoMosaicChip.cpp
    GeoMosaicDlg.cpp
    ModuleManager.cpp
    MosaicManager.cpp
) 
set (MOC_HEADERS
    GeoMosaicDlg.h
)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
include_directories(${Opticks_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(GeoMosaic SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(GeoMosaic
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
install(TARGETS GeoMosaic RUNTIME LIBRARY DESTINATION PlugIns)
