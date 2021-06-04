set(HEADER_FILES
    BatchApplication.h
    DesktopServicesImp.h
    ProgressBriefConsole.h
    ProgressConsole.h
)
set(SOURCE_FILES
    BatchApplication.cpp
    DesktopServicesImp.cpp
    Main.cpp
    ProgressBriefConsole.cpp
    ProgressConsole.cpp
    ../App.rc
)
add_executable(OpticksBatch ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(OpticksBatch
    ${OPTICKS_EXECUTABLE_LIBS}
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${YamlCpp_LIBRARIES}
    ${Minizip_LIBRARIES}
    ${Xerces_LIBRARIES}
    ${XQilla_LIBRARIES}
    ${Raptor_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${PTHREADS_LIBRARY}
    ${Qwt5ForQt5_LIBRARY}
)

set_target_properties(OpticksBatch PROPERTIES DEBUG_POSTFIX "d")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(OpticksBatch PROPERTIES OUTPUT_NAME "linuxBatch")
    target_link_libraries(OpticksBatch ${EXPAT_LIBRARIES} -ldl)
endif()
install(TARGETS OpticksBatch RUNTIME DESTINATION bin)
