set (HEADER_FILES
    IntrospectionMethods.h
    OpticksCallbacks.h
    OpticksMethods.h
    XmlRpc.h
    XmlRpcArrayParam.h
    XmlRpcParam.h
    XmlRpcStructParam.h
)
set (SOURCE_FILES
    ModuleManager.cpp
    OpticksCallbacks.cpp
    OpticksMethods.cpp
    XmlRpc.cpp
    XmlRpcCallback.cpp
    XmlRpcServer.cpp
) 
set (MOC_HEADERS
    XmlRpcCallback.h
    XmlRpcServer.h
)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Opticks_ServerUtilities_INCLUDE_DIR})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${ehs_INCLUDE_DIRS} ${pcre_INCLUDE_DIRS} ${pme_INCLUDE_DIRS})
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
source_group(moc FILES ${MOC_OUTFILES})
add_library(XmlRpc SHARED ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
target_link_libraries(XmlRpc
    ${Opticks_LIBRARIES}
    ${Opticks_ServerUtilities_LIBRARY}
    ${ehs_LIBRARIES}
    ${pcre_LIBRARIES}
    ${pme_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
if(WIN32)
    install(TARGETS XmlRpc RUNTIME DESTINATION PlugIns)
else()
    install(TARGETS XmlRpc LIBRARY DESTINATION PlugIns)
endif()
