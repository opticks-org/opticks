find_path(Ossim_INCLUDE_DIR ossim/ossimConfig.h)

find_library(Ossim_LIBRARY_RELEASE NAMES ossim)
find_library(Ossim_LIBRARY_DEBUG NAMES ossimd)

include(SelectLibraryConfigurations)
select_library_configurations(Ossim)

set(Ossim_INCLUDE_DIRS ${Ossim_INCLUDE_DIR})
mark_as_advanced(Ossim_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ossim REQUIRED_VARS Ossim_INCLUDE_DIR Ossim_LIBRARY)
set(Ossim_FOUND ${OSSIM_FOUND})
