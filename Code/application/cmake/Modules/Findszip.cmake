find_path(szip_INCLUDE_DIR szlib.h PATH_SUFFIXES szip)
if(szip_INCLUDE_DIR AND EXISTS "${szip_INCLUDE_DIR}/szlib.h")
	file(STRINGS "${szip_INCLUDE_DIR}/szlib.h" szip_Parsed_Version REGEX "^#define SZLIB_VERSION +\".+\"$")
    string(REGEX REPLACE "^.*SZLIB_VERSION +\"([0-9]+).*$" "\\1" szip_VERSION_MAJOR "${szip_Parsed_Version}")
    string(REGEX REPLACE "^.*SZLIB_VERSION +\"[0-9]+\\.([0-9]+).*$" "\\1" szip_VERSION_MINOR "${szip_Parsed_Version}")

    set(szip_VERSION_STRING "${szip_VERSION_MAJOR}.${szip_VERSION_MINOR}")
    set(szip_MAJOR_VERSION "${szip_VERSION_MAJOR}")
    set(szip_MINOR_VERSION "${szip_VERSION_MINOR}")
endif()

find_library(szip_LIBRARY_RELEASE NAMES szlibdll szip sz)
find_library(szip_LIBRARY_DEBUG NAMES szlibdlld szipd)

include(SelectLibraryConfigurations)
select_library_configurations(szip)

set(szip_INCLUDE_DIRS ${szip_INCLUDE_DIR})
mark_as_advanced(szip_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(szip REQUIRED_VARS szip_INCLUDE_DIR szip_LIBRARY VERSION_VAR szip_VERSION_STRING)
set(szip_FOUND ${SZIP_FOUND})
