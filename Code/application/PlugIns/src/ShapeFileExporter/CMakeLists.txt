set (HEADER_FILES
   AddFeatureDlg.h
   AddFieldDlg.h
   FeatureClassDlg.h
   Feature.h
   ShapeFileExporter.h
   ShapeFile.h
   ShapeFileOptionsWidget.h
   ShapeFileTypes.h
   )
set (SOURCE_FILES
   AddFeatureDlg.cpp
   AddFieldDlg.cpp
   FeatureClassDlg.cpp
   Feature.cpp
   ModuleManager.cpp
   ShapeFile.cpp
   ShapeFileExporter.cpp
   ShapeFileOptionsWidget.cpp
   ShapeFileTypes.cpp
   ) 
set (MOC_HEADERS
   AddFeatureDlg.h
   AddFieldDlg.h
   FeatureClassDlg.h
   Feature.h
   ShapeFileExporter.h
   ShapeFile.h
   ShapeFileOptionsWidget.h
   ShapeFileTypes.h
   )
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${shapelib_INCLUDE_DIRS} ${Xerces_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(ShapeFileExporter SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(ShapeFileExporter
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${shapelib_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS ShapeFileExporter RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS ShapeFileExporter RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS ShapeFileExporter LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS ShapeFileExporter LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
