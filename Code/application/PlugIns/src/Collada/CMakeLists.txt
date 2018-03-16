if(OpenCollada_FOUND)
    set (HEADER_FILES
        ColladaExporter.h
        ColladaImporter.h
        ColladaStreamReader.h
        ColladaStreamWriter.h
        ColladaUtil.h
    )
    set (SOURCE_FILES
        ColladaExporter.cpp
        ColladaImporter.cpp
        ColladaStreamReader.cpp
        ColladaStreamWriter.cpp
        ModuleManager.cpp
    ) 
    include(${QT_USE_FILE})
    add_definitions(-DAPPLICATION_XERCES)
    include_directories(${Opticks_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS} ${OpenCollada_INCLUDE_DIRS} ${Xerces_INCLUDE_DIRS})
    qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
    source_group(moc FILES ${MOC_OUTFILES})
    add_library(Collada SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
    target_link_libraries(Collada
        ${Opticks_LIBRARIES}
        ${OpenCollada_LIBRARIES}
        ${Xerces_LIBRARIES}
        ${OpenCollada_LIBRARIES}
    )

    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4201" CACHE STRING "Compiler flags" FORCE)
    endif()
    install(TARGETS Collada RUNTIME DESTINATION PlugIns)

endif()
