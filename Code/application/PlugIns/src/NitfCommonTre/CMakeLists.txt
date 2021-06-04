set (HEADER_FILES
    NitfAcftaParser.h
    NitfAcftbParser.h
    NitfAimidaParser.h
    NitfAimidbParser.h
    NitfBandsaParser.h
    NitfBandsbParser.h
    NitfBlockaParser.h
    NitfCmetaaParser.h
    NitfEngrdaParser.h
    NitfExoptaParser.h
    NitfExpltaParser.h
    NitfExpltbParser.h
    NitfIchipbParser.h
    NitfMensraParser.h
    NitfMensrbParser.h
    NitfMod26aParser.h
    NitfMpd26aParser.h
    NitfPatchaParser.h
    NitfPatchbParser.h
    NitfRadsdaParser.h
    NitfReflnaParser.h
    NitfRpc00aParser.h
    NitfRpc00bParser.h
    NitfRpcParser.h
    NitfSectgaParser.h
    NitfSensraParser.h
    NitfSensrbParser.h
    NitfStdidbParser.h
    NitfStdidcParser.h
    NitfUse00aParser.h
    NitfUse26aParser.h
)
set (SOURCE_FILES
    EngrdaWidget.cpp
    ModuleManager.cpp
    NitfAcftaParser.cpp
    NitfAcftbParser.cpp
    NitfAimidaParser.cpp
    NitfAimidbParser.cpp
    NitfBandsaParser.cpp
    NitfBandsbParser.cpp
    NitfBlockaParser.cpp
    NitfCmetaaParser.cpp
    NitfEngrdaParser.cpp
    NitfExoptaParser.cpp
    NitfExpltaParser.cpp
    NitfExpltbParser.cpp
    NitfIchipbParser.cpp
    NitfMensraParser.cpp
    NitfMensrbParser.cpp
    NitfMod26aParser.cpp
    NitfMpd26aParser.cpp
    NitfPatchaParser.cpp
    NitfPatchbParser.cpp
    NitfRadsdaParser.cpp
    NitfReflnaParser.cpp
    NitfRpc00aParser.cpp
    NitfRpc00bParser.cpp
    NitfRpcParser.cpp
    NitfSectgaParser.cpp
    NitfSensraParser.cpp
    NitfSensrbParser.cpp
    NitfStdidbParser.cpp
    NitfStdidcParser.cpp
    NitfUse00aParser.cpp
    NitfUse26aParser.cpp
) 
set (MOC_HEADERS
    EngrdaWidget.h
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Ossim_INCLUDE_DIRS})
include_directories(${Opticks_NitfPlugInLib_INCLUDE_DIR})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(NitfCommonTre SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(NitfCommonTre
    ${Opticks_LIBRARIES}
    ${Opticks_NitfPlugInLib_LIBRARY}
    ${QT_LIBRARIES}    
    ${Ossim_LIBRARIES}
    ${Xerces_LIBRARIES}
)
if(MSVC)
    set_target_properties(NitfCommonTre PROPERTIES
        LINK_FLAGS "/ignore:4006 /force:multiple")
endif()
if(WIN32)
    install(TARGETS NitfCommonTre RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS NitfCommonTre RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS NitfCommonTre LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS NitfCommonTre LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
