set (HEADER_FILES
    BmpDetails.h
    GeoTIFFExporter.h
    GeoTIFFImporter.h
    GeoTiffPage.h
    GeoTiffPager.h
    Jpeg2000Exporter.h
    Jpeg2000Utilities.h
    JpegDetails.h
    JpegExportOptionsWidget.h
    PicturesExporter.h
    PicturesPlotWidgetExporter.h
    PicturesViewExporter.h
    PngDetails.h
    PostScriptExporter.h
    QuickbirdIsd.h
    TiffDetails.h
    TiffExportOptionsWidget.h
)
set (SOURCE_FILES
    BmpDetails.cpp
    GeoTIFFExporter.cpp
    GeoTIFFImporter.cpp
    GeoTiffPage.cpp
    GeoTiffPager.cpp
    Jpeg2000Exporter.cpp
    Jpeg2000Utilities.cpp
    JpegDetails.cpp
    JpegExportOptionsWidget.cpp
    ModuleManager.cpp
    OptionsBmpExporter.cpp
    OptionsJpegExporter.cpp
    OptionsPngExporter.cpp
    OptionsTiffExporter.cpp
    OptionsTiffImporter.cpp
    PicturesExporter.cpp
    PicturesPlotWidgetExporter.cpp
    PicturesViewExporter.cpp
    PngDetails.cpp
    PostScriptExporter.cpp
    QuickbirdIsd.cpp
    TiffDetails.cpp
    TiffExportOptionsWidget.cpp
) 
set (MOC_HEADERS
    OptionsBmpExporter.h
    OptionsJpegExporter.h
    OptionsPngExporter.h
    OptionsTiffExporter.h
    OptionsTiffImporter.h
)
if(OpenJpeg_FOUND)
    set (JPEG2000_HEADER_FILES
    Jpeg2000Importer.h
    Jpeg2000Page.h
    Jpeg2000Pager.h
    )
    set (JPEG2000_SOURCE_FILES
    Jpeg2000Importer.cpp
    Jpeg2000Page.cpp
    Jpeg2000Pager.cpp
    ) 
endif()
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${geotiff_INCLUDE_DIRS})
include_directories(${proj_INCLUDE_DIRS})
include_directories(${tiff_INCLUDE_DIRS})
if(OpenJpeg_FOUND)
    include_directories(${OpenJpeg_INCLUDE_DIRS})
endif()
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(Pictures SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES} ${JPEG2000_HEADER_FILES} ${JPEG2000_SOURCE_FILES})
target_link_libraries(Pictures
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${geotiff_LIBRARIES}
    ${proj_LIBRARIES}
    ${tiff_LIBRARIES}
    ${PTHREADS_LIBRARY}
)
if(OpenJpeg_FOUND)
    target_link_libraries(Pictures
        ${OpenJpeg_LIBRARIES})
endif()
if(WIN32)
    install(TARGETS Pictures RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Pictures RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Pictures LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Pictures LIBRARY DESTINATION PlugIns CONFIGURATIONS Debug)
endif()
