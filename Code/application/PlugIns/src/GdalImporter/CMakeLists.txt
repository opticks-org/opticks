set (HEADER_FILES
    GdalImporter.h
    GdalRasterPager.h
)
set (SOURCE_FILES
    GdalImporter.cpp
    GdalRasterPager.cpp
    ModuleManager.cpp
) 
include(${QT_USE_FILE})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Gdal_INCLUDE_DIRS})
add_library(GdalImporter SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(GdalImporter
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${Gdal_LIBRARIES}    
    ${Xerces_LIBRARIES}
	${PTHREADS_LIBRARY}
)
