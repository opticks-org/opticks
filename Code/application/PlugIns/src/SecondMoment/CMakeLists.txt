set (HEADER_FILES
    SecondMoment.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    SecondMoment.cpp
    SecondMomentGui.cpp
) 
set (MOC_HEADERS
    SecondMomentGui.h 
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Ossim_INCLUDE_DIRS})
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(SecondMoment SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(SecondMoment
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Ossim_LIBRARIES}
    ${Xerces_LIBRARIES}
)
