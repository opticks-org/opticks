set (HEADER_FILES
    FeatureLayerImporter.h
    PropertiesFeatureClass.h
    QueryOptions.h
    ShapeFileImporter.h
)
set (SOURCE_FILES
    ConnectionParametersWidget.cpp
    FeatureClass.cpp
    FeatureClassWidget.cpp
    FeatureLayerImporter.cpp
    FeatureManager.cpp
    FeatureMenuEditorDlg.cpp
    FeatureProxyConnector.cpp
    ListInspectorWidget.cpp
    ModuleManager.cpp
    PropertiesFeatureClass.cpp
    QueryBuilderWidget.cpp
    QueryOptions.cpp
    QueryOptionsWidget.cpp
    ShapeFileImporter.cpp
    ShapelibProxy.cpp
) 
set (MOC_HEADERS
    ConnectionParametersWidget.h
    FeatureClass.h
    FeatureClassWidget.h
    FeatureManager.h
    FeatureMenuEditorDlg.h
    FeatureProxyConnector.h
    ListInspectorWidget.h
    QueryBuilderWidget.h
    QueryOptionsWidget.h
    ShapelibProxy.h
)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${shapelib_INCLUDE_DIRS})
include_directories("${Opticks_SOURCE_DIR}/ArcProxyLib")
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(GeographicFeatures SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(GeographicFeatures
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${shapelib_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
