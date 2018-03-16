find_path(tiff_INCLUDE_DIR tiff.h)
if(tiff_INCLUDE_DIR AND EXISTS "${tiff_INCLUDE_DIR}/tiffvers.h")
    file(STRINGS "${tiff_INCLUDE_DIR}/tiffvers.h" tiff_Parsed_Version REGEX "^#define TIFFLIB_VERSION +[0-9]+.*$")
    string(REGEX REPLACE "^.*TIFFLIB_VERSION +([0-9]+).*$" "\\1" tiff_VERSION_MAJOR "${tiff_Parsed_Version}")

    set(tiff_VERSION_STRING "${tiff_VERSION_MAJOR}")
    set(tiff_MAJOR_VERSION "${tiff_VERSION_MAJOR}")
endif()

find_library(tiff_LIBRARY_RELEASE NAMES libtiff tiff)
find_library(tiff_LIBRARY_DEBUG NAMES libtiffd)

include(SelectLibraryConfigurations)
select_library_configurations(tiff) #sets tiff_LIBRARY using tiff_LIBRARY_DEBUG and tiff_LIBRARY_RELEASE

set(tiff_INCLUDE_DIRS ${tiff_INCLUDE_DIR})
mark_as_advanced(tiff_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tiff REQUIRED_VARS tiff_INCLUDE_DIR tiff_LIBRARY VERSION_VAR tiff_VERSION_STRING)
set(tiff_FOUND ${TIFF_FOUND})
