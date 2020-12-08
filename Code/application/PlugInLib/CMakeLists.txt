set(HEADER_FILES
    AlgorithmShell.h
    ExecutableShell.h
    ExporterShell.h
    GeoreferenceShell.h
    ImporterShell.h
    InterpreterManagerShell.h
    OptionShell.h
    PlugInShell.h
    PointCloudPagerShell.h
    PropertiesShell.h
    RasterElementImporterShell.h
    RasterPagerShell.h
    ViewerShell.h
    WizardShell.h
)
set(SOURCE_FILES
    AlgorithmShell.cpp
    DataAccessor.cpp
    DockWindowShell.cpp
    ExecutableShell.cpp
    ExporterShell.cpp
    GeoreferenceShell.cpp
    ImporterShell.cpp
    InterpreterManagerShell.cpp
    ModuleManagerCommon.cpp
    ModuleShell.cpp
    OptionShell.cpp
    PlugInShell.cpp
    PointCloudAccessor.cpp
    PointCloudPagerShell.cpp
    PropertiesShell.cpp
    RasterElementImporterShell.cpp
    RasterPagerShell.cpp
    ViewerShell.cpp
    WizardShell.cpp
)
set (MOC_HEADERS
    DockWindowShell.h
)
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
qt5_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})
source_group(moc FILES ${MOC_OUTFILES})
add_library(PlugInLib STATIC ${SOURCE_FILES} ${MOC_HEADERS} ${MOC_OUTFILES} ${HEADER_FILES})
set_target_properties(PlugInLib PROPERTIES OUTPUT_NAME opticks-pluginlib)
if(NOT MSVC)
    #these libraries should be fixed so that they are not circular
    target_link_libraries(PlugInLib PlugInUtilities)
endif()
install(TARGETS PlugInLib DESTINATION lib)
