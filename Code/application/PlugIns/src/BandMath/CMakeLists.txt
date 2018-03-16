set (HEADER_FILES
    BandMath.h
    bmathfuncs.h
    bm.ui.h
)
set (SOURCE_FILES
    BandMath.cpp
    bm.cpp
    bmathfuncs.cpp
    mbox.cpp
    ModuleManager.cpp
) 
set (MOC_HEADERS
    bm.h
    mbox.h
)
include(${QT_USE_FILE})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS} ${Xerces_INCLUDE_DIRS})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(BandMath SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(BandMath
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS BandMath RUNTIME DESTINATIONS PlugIns)
else()
    install(TARGETS BandMath LIBRARY DESTINATION PlugIns)
endif()
