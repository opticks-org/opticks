set (HEADER_FILES
    PCA.h
)
set (SOURCE_FILES
    EigenPlotDlg.cpp
    ModuleManager.cpp
    PCA.cpp
    PcaDlg.cpp
    StatisticsDlg.cpp
) 
set (MOC_HEADERS
    EigenPlotDlg.h
    PcaDlg.h
    StatisticsDlg.h
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Ossim_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${Qwt5ForQt4_INCLUDE_DIRS})
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(Pca SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(Pca
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Qwt5ForQt4_LIBRARY}
    ${Ossim_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS Pca RUNTIME DESTINATIONS PlugIns)
else()
    install(TARGETS Pca LIBRARY DESTINATION PlugIns)
endif()
