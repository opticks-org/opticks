set (HEADER_FILES
    Mie4NitfImporter.h
    Mie4NitfPager.h
    NitfDesExporter.h
    NitfExporter.h
    NitfImporter.h
    NitfPager.h
    NitfPropertiesManager.h
    NitfUnknownTreParser.h
    OssimAppMemorySource.h
    RpcGeoreference.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    Mie4NitfImporter.cpp
    Mie4NitfPager.cpp
    NitfDesExporter.cpp
    NitfExporter.cpp
    NitfImporter.cpp
    NitfPager.cpp
    NitfPropertiesManager.cpp
    NitfUnknownTreParser.cpp
    OptionsNitfExporter.cpp
    OssimAppMemorySource.cpp
    OssimServices.cpp
    RpcGeoreference.cpp
    RpcGui.cpp
)

if(NOT JPEG2000_SUPPORT)
   message(STATUS "openjpeg.h NOTFOUND: cannot include Nitf/Mie4NitfJpeg2000Pager")
else()
   list(APPEND SOURCE_FILES Mie4NitfJpeg2000Pager.cpp)
   list(APPEND HEADER_FILES Mie4NitfJpeg2000Pager.h)
endif()

set (MOC_HEADERS
    OptionsNitfExporter.h
    RpcGui.h
)
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${Ossim_INCLUDE_DIRS})
include_directories(${Opticks_NitfPlugInLib_INCLUDE_DIR})
include_directories(${OpenJpeg_INCLUDE_DIRS})
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(Nitf SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(Nitf
    ${Opticks_LIBRARIES}
    ${Opticks_NitfPlugInLib_LIBRARY}
    ${QT_LIBRARIES}    
    ${Ossim_LIBRARIES}
    ${Xerces_LIBRARIES}
    ${OpenJpeg_LIBRARIES}
    ${PTHREADS_LIBRARY}
)
if(MSVC)
    set_target_properties(Nitf PROPERTIES
        LINK_FLAGS "/ignore:4006 /force:multiple")
endif()
if(WIN32)
    install(TARGETS Nitf RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Nitf RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS Nitf LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS Nitf LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
