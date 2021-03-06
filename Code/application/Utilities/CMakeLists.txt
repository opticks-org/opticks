set(HEADER_FILES
    ApplicationServicesImp.h
    BadValuesAdapter.h
    BadValuesImp.h
    ConfigurationSettingsImp.h
    DataValueWrapper.h
    DynamicObjectAdapter.h
    ExecutableAgentAdapter.h
    ExecutableAgentImp.h
    ExportAgentAdapter.h
    ExportAgentImp.h
    FileFinderImp.h
    FilenameImp.h
    GeocoordLinkFunctor.h
    GeoreferenceDescriptorAdapter.h
    GeoreferenceDescriptorImp.h
    ImportAgentAdapter.h
    ImportAgentImp.h
    ImportDescriptorImp.h
    MessageLogAdapter.h
    MessageLogImp.h
    MessageLogMgrImp.h
    MruFile.h
    ObjectFactoryImp.h
    ProgressAdapter.h
    ProgressImp.h
    SerializableImp.h
    SessionItemDeserializerImp.h
    SessionItemImp.h
    SessionItemSerializerImp.h
    SessionManagerImp.h
    SettableSessionItemAdapter.h
    ThreadSafeProgressAdapter.h
    ThreadSafeProgressImp.h
    UtilityServicesImp.h
    WavelengthsImp.h
)
set(SOURCE_FILES
    ApplicationServicesImp.cpp
    BadValuesAdapter.cpp
    BadValuesImp.cpp
    ConfigurationSettingsImp.cpp
    DataVariantFactoryImp.cpp
    DateTimeImp.cpp
    DynamicObjectImp.cpp
    ExecutableAgentImp.cpp
    ExportAgentImp.cpp
    FileFinderImp.cpp
    FilenameImp.cpp
    GeocoordLinkFunctor.cpp
    GeoreferenceDescriptorAdapter.cpp
    GeoreferenceDescriptorImp.cpp
    ImportAgentImp.cpp
    ImportDescriptorImp.cpp
    MessageLogAdapter.cpp
    MessageLogImp.cpp
    MessageLogMgrImp.cpp
    MruFile.cpp
    ObjectFactoryImp.cpp
    ProgressImp.cpp
    SerializableImp.cpp
    SessionItemDeserializerImp.cpp
    SessionItemImp.cpp
    SessionItemSerializerImp.cpp
    SessionManagerImp.cpp
    SettableSessionItemAdapter.cpp
    ThreadSafeProgressImp.cpp
    UtilityServicesImp.cpp
    WavelengthsImp.cpp
)
if(ONE_OPTICKS_LIB)
    set_property(DIRECTORY PROPERTY ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES})
else()
    add_library(Utilities STATIC ${SOURCE_FILES} ${HEADER_FILES})
endif()
