set (HEADER_FILES
    AnnotationImagePalette.h
    FlattenAnnotationLayer.h
)
set (SOURCE_FILES
    AnnotationImagePalette.cpp
    AnnotationImagePaletteOptions.cpp
    AnnotationImagePaletteWidget.cpp
    FlattenAnnotationLayer.cpp
    ModuleManager.cpp
) 
set (MOC_HEADERS
    AnnotationImagePaletteOptions.h
    AnnotationImagePaletteWidget.h
)
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(AnnotationImagePalette SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(AnnotationImagePalette
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS AnnotationImagePalette RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS AnnotationImagePalette RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS AnnotationImagePalette LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS AnnotationImagePalette LIBRARY DESTINATION PlugIns CONFIGURATIONS Debug)
endif()
