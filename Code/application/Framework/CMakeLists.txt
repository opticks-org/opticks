set(HEADER_FILES
    Application.h
    ArgumentList.h
    CopyrightInformation.h
    InteractiveApplication.h
)
set(SOURCE_FILES
    Application.cpp
    ArgumentList.cpp
    CopyrightInformation.cpp
    InteractiveApplication.cpp
)
set(ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES})
if(ONE_OPTICKS_LIB)
    set_property(DIRECTORY PROPERTY ALL_SOURCES ${ALL_SOURCES})
else()
    add_library(Framework STATIC ${ALL_SOURCES})
endif()
