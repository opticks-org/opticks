set(HEADER_FILES
    ConnectionManager.h
    CoreModuleDescriptor.h
    DynamicModuleImp.h
    ModuleDescriptor.h
    PlugInArgImp.h
    PlugInArgListImp.h
    PlugInDescriptorImp.h
    PlugInManagerServicesImp.h
)
set(SOURCE_FILES
    ConnectionManager.cpp
    CoreModuleDescriptor.cpp
    DynamicModuleImp.cpp
    ModuleDescriptor.cpp
    PlugInArgImp.cpp
    PlugInArgListImp.cpp
    PlugInDescriptorImp.cpp
    PlugInManagerServicesImp.cpp
)
if(ONE_OPTICKS_LIB)
    set_property(DIRECTORY PROPERTY ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES})
else()
    add_library(PlugInManager STATIC ${SOURCE_FILES} ${HEADER_FILES})
endif()


