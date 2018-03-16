find_path(Raptor_INCLUDE_DIR raptor.h)

find_library(Raptor_LIBRARY_RELEASE NAMES raptor)
find_library(Raptor_LIBRARY_DEBUG NAMES raptord)

include(SelectLibraryConfigurations)
select_library_configurations(Raptor) #sets Raptor_LIBRARY using Raptor_LIBRARY_DEBUG and Raptor_LIBRARY_RELEASE

mark_as_advanced(Raptor_INCLUDE_DIR)
set(Raptor_INCLUDE_DIRS ${Raptor_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Raptor REQUIRED_VARS Raptor_INCLUDE_DIR Raptor_LIBRARY)

set(Raptor_FOUND ${RAPTOR_FOUND})
