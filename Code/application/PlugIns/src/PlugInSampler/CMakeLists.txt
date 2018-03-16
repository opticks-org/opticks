set (HEADER_FILES
    AnyPlugIn.h
    BackgroundTest.h
    CustomElementData.h
    CustomElementPlugIn.h
    CustomMenuPlugIn.h
    DummyCustomAlgorithm.h
    DummyCustomImporter.h
    MessageLogTest.h
    SampleRasterElementImporter.h
    Scriptor.h
)
set (SOURCE_FILES
    AnyPlugIn.cpp
    BackgroundTest.cpp
    CustomElementPlugIn.cpp
    CustomMenuPlugIn.cpp
    DummyCustomAlgorithm.cpp
    DummyCustomImporter.cpp
    MessageLogTest.cpp
    ModuleManager.cpp
    SampleRasterElementImporter.cpp
    Scriptor.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(PlugInSampler SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(PlugInSampler
    ${Opticks_LIBRARIES}
    ${Xerces_LIBRARIES}
	${PTHREADS_LIBRARY}
)
install(TARGETS PlugInSampler RUNTIME LIBRARY DESTINATION PlugIns)
