set (HEADER_FILES
    ApiUtilities.h
    DynamicTypes.h
    GeographicData.h
    Metadata.h
    RasterData.h
    SimpleAnimation.h
    SimpleAoi.h
    SimpleApiErrors.h
    SimpleExecution.h
    SimpleSignature.h
    SimpleViews.h
)
set (SOURCE_FILES
    ApiUtilities.cpp
    DynamicTypes.cpp
    GeographicData.cpp
    Metadata.cpp
    RasterData.cpp
    SimpleAnimation.cpp
    SimpleAoi.cpp
    SimpleApiErrors.cpp
    SimpleExecution.cpp
    SimpleSignature.cpp
    SimpleViews.cpp
) 
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Opticks_SOURCE_DIR}/Wizard)
add_definitions(-DAPPLICATION_XERCES)
include_directories(${Xerces_INCLUDE_DIRS})
add_library(SimpleApiLib SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(SimpleApiLib
    ${Opticks_LIBRARIES}
    ${Opticks_WizardPrivate_LIBRARY} 
    ${QT_LIBRARIES}    
    ${Xerces_LIBRARIES}
)
