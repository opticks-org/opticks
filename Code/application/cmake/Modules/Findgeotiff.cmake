find_path(geotiff_INCLUDE_DIR geotiff.h PATH_SUFFIXES geotiff libgeotiff)

if(geotiff_INCLUDE_DIR AND EXISTS "${geotiff_INCLUDE_DIR}/geotiff.h")
    file(STRINGS "${geotiff_INCLUDE_DIR}/geotiff.h" geotiff_Parsed_Version REGEX "^#define LIBGEOTIFF_VERSION +[0-9]+.*$")
    string(SUBSTRING ${geotiff_Parsed_Version} 27 -1 geotiff_Parsed_Version)
    math(EXPR geotiff_MAJOR_VERSION "${geotiff_Parsed_Version} / 1000")
    math(EXPR geotiff_MINOR_VERSION "${geotiff_Parsed_Version} % 1000 / 100")
    math(EXPR geotiff_PATCH_VERSION "${geotiff_Parsed_Version} % 1000 % 100 / 10")
    set(geotiff_VERSION_STRING "${geotiff_MAJOR_VERSION}.${geotiff_MINOR_VERSION}.${geotiff_PATCH_VERSION}")
endif()

find_library(geotiff_LIBRARY_RELEASE NAMES geotiff)
find_library(geotiff_LIBRARY_DEBUG NAMES geotiffd)

include(SelectLibraryConfigurations)
select_library_configurations(geotiff) #sets geotiff_LIBRARY using geotiff_LIBRARY_DEBUG and geotiff_LIBRARY_RELEASE

mark_as_advanced(geotiff_INCLUDE_DIR)
set(geotiff_INCLUDE_DIRS ${geotiff_INCLUDE_DIR})

set(geotiff_LIBRARIES ${geotiff_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(geotiff REQUIRED_VARS geotiff_INCLUDE_DIR geotiff_LIBRARY VERSION_VAR geotiff_VERSION_STRING)

set(geotiff_FOUND ${GEOTIFF_FOUND})
