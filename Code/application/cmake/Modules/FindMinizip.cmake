find_path(Minizip_INCLUDE_DIR unzip.h PATH_SUFFIXES minizip)

find_library(Minizip_LIBRARY_RELEASE NAMES minizip)
find_library(Minizip_LIBRARY_DEBUG NAMES minizipd)

include(SelectLibraryConfigurations)
select_library_configurations(Minizip) #sets Minizip_LIBRARY using Minizip_LIBRARY_DEBUG and Minizip_LIBRARY_RELEASE

set(Minizip_INCLUDE_DIRS ${Minizip_INCLUDE_DIR})
mark_as_advanced(Minizip_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Minizip REQUIRED_VARS Minizip_INCLUDE_DIR Minizip_LIBRARY)
set(Minizip_FOUND ${MINIZIP_FOUND})
