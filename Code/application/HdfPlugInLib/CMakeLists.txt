set (HEADER_FILES
    Hdf4Attribute.h
    Hdf4Dataset.h
    Hdf4Element.h
    Hdf4File.h
    Hdf4Group.h
    Hdf4ImporterShell.h
    Hdf4Pager.h
    Hdf4Utilities.h
    Hdf5Attribute.h
    Hdf5CustomReader.h
    Hdf5CustomWriter.h
    Hdf5Data.h
    Hdf5Dataset.h
    Hdf5Element.h
    Hdf5File.h
    Hdf5Group.h
    Hdf5ImporterShell.h
    Hdf5IncrementalReader.h
    Hdf5IncrementalWriter.h
    Hdf5Node.h
    Hdf5Pager.h
    Hdf5PagerFileHandle.h
    Hdf5Resource.h
    Hdf5Utilities.h
    HdfImporterShell.h
    HdfPager.h
    HdfUtilities.h
)
set (SOURCE_FILES
    Hdf4Attribute.cpp
    Hdf4Dataset.cpp
    Hdf4Element.cpp
    Hdf4File.cpp
    Hdf4Group.cpp
    Hdf4ImporterShell.cpp
    Hdf4Pager.cpp
    Hdf4Utilities.cpp
    Hdf5Attribute.cpp
    Hdf5Data.cpp
    Hdf5Dataset.cpp
    Hdf5Element.cpp
    Hdf5File.cpp
    Hdf5Group.cpp
    Hdf5ImporterShell.cpp
    Hdf5IncrementalReader.cpp
    Hdf5Node.cpp
    Hdf5Pager.cpp
    Hdf5ReadersWriters.cpp
    Hdf5Utilities.cpp
    HdfImporterShell.cpp
    HdfPager.cpp
    HdfUtilities.cpp
) 
add_definitions(${Hdf5_DEFINITIONS})
include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Hdf4_INCLUDE_DIRS} ${Hdf5_INCLUDE_DIRS})
add_library(HdfPlugInLib STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(HdfPlugInLib PROPERTIES OUTPUT_NAME opticks-hdfpluginlib)
install(TARGETS HdfPlugInLib DESTINATION lib)
