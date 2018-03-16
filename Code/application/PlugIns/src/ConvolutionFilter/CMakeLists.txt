set (HEADER_FILES
    ConvolutionFilterShell.h
    ConvolutionMatrixEditor.h
    MorphologicalFilter.h
)
set (SOURCE_FILES
    ConvolutionFilterShell.cpp
    ConvolutionMatrixEditor.cpp
    ConvolutionMatrixWidget.cpp
    GetConvolveParametersDialog.cpp
    ModuleManager.cpp
    MorphologicalFilter.cpp
) 
set (MOC_HEADERS
    ConvolutionMatrixWidget.h
    GetConvolveParametersDialog.h
)
set (UI_FILES
    ConvolutionMatrixWidget.ui
)
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OpenCv_INCLUDE_DIRS})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
qt4_wrap_ui(UI_OUTFILES ${UI_FILES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
source_group(moc FILES ${MOC_OUTFILES})
include_directories(${Ossim_INCLUDE_DIRS})
add_library(ConvolutionFilter SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES} ${UI_OUTFILES})
target_link_libraries(ConvolutionFilter
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Ossim_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${OpenCv_LIBRARIES}
    ${PTHREADS_LIBRARY}
)
if(WIN32)
    install(TARGETS ConvolutionFilter RUNTIME DESTINATIONS PlugIns)
else()
    install(TARGETS ConvolutionFilter LIBRARY DESTINATION PlugIns)
endif()
