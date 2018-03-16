find_path(pme_INCLUDE_DIR pme.h)

find_library(pme_LIBRARY_RELEASE NAMES pme)
find_library(pme_LIBRARY_DEBUG NAMES pmeD)

include(SelectLibraryConfigurations)
select_library_configurations(pme)

mark_as_advanced(pme_INCLUDE_DIR)
set(pme_INCLUDE_DIRS ${pme_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pme REQUIRED_VARS pme_INCLUDE_DIR pme_LIBRARY)

set(pme_FOUND ${PME_FOUND})
