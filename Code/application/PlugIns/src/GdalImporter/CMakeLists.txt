set (HEADER_FILES
    GdalImporter.h
    GdalRasterPager.h
)
set (SOURCE_FILES
    GdalImporter.cpp
    GdalRasterPager.cpp
    ModuleManager.cpp
) 
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${GDAL_INCLUDE_DIR})
add_library(GdalImporter SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(GdalImporter
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}    
    ${GDAL_LIBRARY}
    ${Xerces_LIBRARIES}
    ${PTHREADS_LIBRARY}
)
if(WIN32)
    install(TARGETS GdalImporter RUNTIME DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS GdalImporter RUNTIME DESTINATION PlugIns/debug CONFIGURATIONS Debug)
else()
    install(TARGETS GdalImporter LIBRARY DESTINATION PlugIns CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
    install(TARGETS GdalImporter LIBRARY DESTINATION "PlugIns/debug" CONFIGURATIONS Debug)
endif()
