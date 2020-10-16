find_path(shapelib_INCLUDE_DIR shapefil.h)
find_library(shapelib_LIBRARY_RELEASE NAMES shapelib shp)
find_library(shapelib_LIBRARY_DEBUG NAMES shapelibd)

include(SelectLibraryConfigurations)
select_library_configurations(shapelib) #sets shapelib_LIBRARY using shapelib_LIBRARY_DEBUG and shapelib_LIBRARY_RELEASE

set(shapelib_INCLUDE_DIRS ${shapelib_INCLUDE_DIR})
mark_as_advanced(shapelib_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(shapelib REQUIRED_VARS shapelib_INCLUDE_DIR shapelib_LIBRARY)
set(shapelib_FOUND ${SHAPELIB_FOUND})
