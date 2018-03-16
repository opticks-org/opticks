set (HEADER_FILES
    AdvancedOptionsWidget.h
    AviExporter.h
    MjpegExporter.h
    MovieExporter.h
    MovieExportOptionsWidget.h
    Mpeg1Exporter.h
    OptionsMovieExporter.h
)
set (SOURCE_FILES
    AdvancedOptionsWidget.cpp
    AviExporter.cpp
    BitrateWidget.cpp
    FramerateWidget.cpp
    MjpegExporter.cpp
    ModuleManager.cpp
    MovieExporter.cpp
    MovieExportOptionsWidget.cpp
    Mpeg1Exporter.cpp
    OptionsMovieExporter.cpp
    ViewResolutionWidget.cpp
) 
set (MOC_HEADERS
    BitrateWidget.h
    FramerateWidget.h
    ViewResolutionWidget.h
)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${avutil_INCLUDE_DIRS})
include_directories(${avformat_INCLUDE_DIRS})
include_directories(${avcodec_INCLUDE_DIRS})
add_definitions(${avutil_DEFINITIONS})
add_definitions(${avformat_DEFINITIONS})
add_definitions(${avcodec_DEFINITIONS})
add_library(MovieExporter SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(MovieExporter
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${avutil_LIBRARIES}
    ${avformat_LIBRARIES}
    ${avcodec_LIBRARIES}
)
if(WIN32)
    install(TARGETS MovieExporter RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS MovieExporter RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS MovieExporter LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS MovieExporter LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
