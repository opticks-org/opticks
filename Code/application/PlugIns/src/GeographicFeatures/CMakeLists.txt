set (HEADER_FILES
   AddQueriesDlg.h
   AlphaNumericSortFilterProxyModel.h
   ConnectionParametersWidget.h
   CoordinateTransformation.h
   DisplayQueryOptions.h
   DisplaySelectionWidget.h
   FeatureClass.h
   FeatureClassWidget.h
   FeatureLayerExporter.h
   FeatureLayerImporter.h
   FeatureManager.h
   FeatureMenuEditorDlg.h
   FeatureProxyConnector.h
   FeatureQueryOptions.h
   FeatureTable.h
   GeographicFeaturesTabs.h
   GeographicFeaturesWidget.h
   GeographicFeaturesWindow.h
   ListInspectorWidget.h
   OptionsGeographicFeatures.h
   PropertiesFeatureClass.h
   QueryBuilderWidget.h
   QueryOptions.h
   QueryOptionsWidget.h
   ShapeFileImporter.h
   ShapelibProxy.h
   )
set (SOURCE_FILES
   AddQueriesDlg.cpp
   AlphaNumericSortFilterProxyModel.cpp
   ConnectionParametersWidget.cpp
   CoordinateTransformation.cpp
   DisplayQueryOptions.cpp
   DisplaySelectionWidget.cpp
   FeatureClass.cpp
   FeatureClassWidget.cpp
   FeatureLayerExporter.cpp
   FeatureLayerImporter.cpp
   FeatureManager.cpp
   FeatureMenuEditorDlg.cpp
   FeatureProxyConnector.cpp
   FeatureQueryOptions.cpp
   FeatureTable.cpp
   GeographicFeaturesTabs.cpp
   GeographicFeaturesWidget.cpp
   GeographicFeaturesWindow.cpp
   ListInspectorWidget.cpp
   ModuleManager.cpp
   OptionsGeographicFeatures.cpp
   PropertiesFeatureClass.cpp
   QueryBuilderWidget.cpp
   QueryOptions.cpp
   QueryOptionsWidget.cpp
   ShapeFileImporter.cpp
   ShapelibProxy.cpp
) 
set (MOC_HEADERS
   AddQueriesDlg.h
   AlphaNumericSortFilterProxyModel.h
   ConnectionParametersWidget.h
   CoordinateTransformation.h
   DisplayQueryOptions.h
   DisplaySelectionWidget.h
   FeatureClass.h
   FeatureClassWidget.h
   FeatureLayerExporter.h
   FeatureLayerImporter.h
   FeatureManager.h
   FeatureMenuEditorDlg.h
   FeatureProxyConnector.h
   FeatureQueryOptions.h
   FeatureTable.h
   GeographicFeaturesTabs.h
   GeographicFeaturesWidget.h
   GeographicFeaturesWindow.h
   ListInspectorWidget.h
   OptionsGeographicFeatures.h
   PropertiesFeatureClass.h
   QueryBuilderWidget.h
   QueryOptions.h
   QueryOptionsWidget.h
   ShapeFileImporter.h
   ShapelibProxy.h
)
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
find_package(GDAL REQUIRED)
source_group(moc FILES ${MOC_OUTFILES})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${shapelib_INCLUDE_DIRS})
include_directories("${Opticks_SOURCE_DIR}/ArcProxyLib")
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${GDAL_INCLUDE_DIR})

add_library(GeographicFeatures SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(GeographicFeatures
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${shapelib_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${GDAL_LIBRARY}
)
if(WIN32)
    install(TARGETS GeographicFeatures RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS GeographicFeatures RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS GeographicFeatures LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS GeographicFeatures LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
