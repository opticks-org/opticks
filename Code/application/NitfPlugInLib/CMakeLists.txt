set (HEADER_FILES
    NitfChipConverter.h
    NitfConstants.h
    NitfDesSubheader.h
    NitfFileHeader.h
    NitfHeader.h
    NitfImageSubheader.h
    NitfMetadataParsing.h
    NitfProperties.h
    NitfPropertiesQWidgetWrapper.h
    NitfResource.h
    NitfTreParser.h
    NitfTreParserShell.h
    NitfUtilities.h
)
set (SOURCE_FILES
    NitfChipConverter.cpp
    NitfDesSubheader.cpp
    NitfFileHeader.cpp
    NitfHeader.cpp
    NitfImageSubheader.cpp
    NitfMetadataParsing.cpp
    NitfResource.cpp
    NitfTreParserShell.cpp
    NitfUtilities.cpp
    StubString.cpp
)

if(NOT JPEG2000_SUPPORT)
   message(STATUS "openjpeg.h NOTFOUND: cannot include NitfPlugInLib/NitfImporterShell")
else()
   list(APPEND SOURCE_FILES "NitfImporterShell.cpp")
   list(APPEND HEADER_FILES "NitfImporterShell.h")
endif()

include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Ossim_INCLUDE_DIRS})
add_definitions(-DAPPLICATION_XERCES)
include_directories(${OpenJpeg_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
add_library(NitfPlugInLib STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(NitfPlugInLib PROPERTIES OUTPUT_NAME opticks-nitfpluginlib)
install(TARGETS NitfPlugInLib DESTINATION lib)
