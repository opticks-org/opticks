set (HEADER_FILES
    ConnectedComponents.h
    QtCluster.h
)
set (SOURCE_FILES
    ConnectedComponents.cpp
    ModuleManager.cpp
    QtCluster.cpp
    QtClusterGui.cpp
) 
set (MOC_HEADERS
    QtClusterGui.h
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${OpenCv_INCLUDE_DIRS})
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(ObjectFinding SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(ObjectFinding
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${OpenCv_LIBRARIES}
)
if(WIN32)
    install(TARGETS ObjectFinding RUNTIME DESTINATION PlugIns)
else()
    install(TARGETS ObjectFinding LIBRARY DESTINATION PlugIns)
endif()
