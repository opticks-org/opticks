find_path(ehs_INCLUDE_DIR ehs.h PATH_SUFFIXES ehs)

find_library(ehs_LIBRARY_RELEASE NAMES ehs)
find_library(ehs_LIBRARY_DEBUG NAMES ehsD)

include(SelectLibraryConfigurations)
select_library_configurations(ehs)

mark_as_advanced(ehs_INCLUDE_DIR)
set(ehs_INCLUDE_DIRS ${ehs_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ehs REQUIRED_VARS ehs_INCLUDE_DIR ehs_LIBRARY)

set(ehs_FOUND ${EHS_FOUND})
