set (HEADER_FILES
    BandBinning.h
    BandBinningUtilities.h
)
set (SOURCE_FILES
    BandBinning.cpp
    BandBinningDlg.cpp
    BandBinningModel.cpp
    BandBinningUtilities.cpp
    ModuleManager.cpp
) 
set (MOC_HEADERS
    BandBinningDlg.h
    BandBinningModel.h
)
include(${QT_USE_FILE})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS} ${Xerces_INCLUDE_DIRS})
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(BandBinning SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(BandBinning
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS BandBinning RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS BandBinning RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS BandBinning LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS BandBinning LIBRARY DESTINATION PlugIns CONFIGURATIONS Debug)
endif()
