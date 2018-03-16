set (HEADER_FILES
    DateTimeReaderWriter.h
    GcpPointReaderWriter.h
    IceExporterShell.h
    IceImporterShell.h
    IcePseudocolorLayerExporter.h
    IcePseudocolorLayerImporter.h
    IceRasterElementExporter.h
    IceRasterElementImporter.h
    IceReader.h
    IceThresholdLayerExporter.h
    IceThresholdLayerImporter.h
    IceUtilities.h
    IceWriter.h
    StatisticsReaderWriter.h
)
set (SOURCE_FILES
    DateTimeReaderWriter.cpp
    GcpPointReaderWriter.cpp
    IceExporterShell.cpp
    IceImporterShell.cpp
    IcePseudocolorLayerExporter.cpp
    IcePseudocolorLayerImporter.cpp
    IceRasterElementExporter.cpp
    IceRasterElementImporter.cpp
    IceReader.cpp
    IceThresholdLayerExporter.cpp
    IceThresholdLayerImporter.cpp
    IceUtilities.cpp
    IceWriter.cpp
    ModuleManager.cpp
    OptionsIceExporter.cpp
    StatisticsReaderWriter.cpp
) 
set (MOC_HEADERS
    OptionsIceExporter.h
)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_definitions(${Hdf5_DEFINITIONS})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Hdf5_INCLUDE_DIRS})
include_directories(${Opticks_HdfPlugInLib_INCLUDE_DIR})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(Ice SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(Ice
    ${Opticks_LIBRARIES}
    ${Opticks_HdfPlugInLib_LIBRARY}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${Hdf5_LIBRARIES}
	${PTHREADS_LIBRARY}
)
if(WIN32)
    install(TARGETS Ice RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Ice RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Ice LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Ice LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
