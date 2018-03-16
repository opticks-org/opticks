find_path(cfitsio_INCLUDE_DIR fitsio.h)
if(cfitsio_INCLUDE_DIR AND EXISTS "${cfitsio_INCLUDE_DIR}/fitsio.h")
    file(STRINGS "${cfitsio_INCLUDE_DIR}/fitsio.h" cfitsio_Parsed_Version REGEX "^#define CFITSIO_VERSION +.+$")
    string(REGEX REPLACE "^.*CFITSIO_VERSION +([0-9]+).*$" "\\1" cfitsio_VERSION_MAJOR "${cfitsio_Parsed_Version}")
    string(REGEX REPLACE "^.*CFITSIO_VERSION +[0-9]\\.([0-9]+).*$" "\\1" cfitsio_VERSION_MINOR "${cfitsio_Parsed_Version}")

    set(cfitsio_VERSION_STRING "${cfitsio_VERSION_MAJOR}.${cfitsio_VERSION_MINOR}")
    set(cfitsio_MAJOR_VERSION "${cfitsio_VERSION_MAJOR}")
    set(cfitsio_MINOR_VERSION "${cfitsio_VERSION_MINOR}")
endif()

find_library(cfitsio_LIBRARY_RELEASE NAMES cfitsio)
find_library(cfitsio_LIBRARY_DEBUG NAMES cfitsiod)

include(SelectLibraryConfigurations)
select_library_configurations(cfitsio) #sets cfitsio_LIBRARY using cfitsio_LIBRARY_DEBUG and cfitsio_LIBRARY_RELEASE

set(cfitsio_INCLUDE_DIRS ${cfitsio_INCLUDE_DIR})
mark_as_advanced(cfitsio_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cfitsio REQUIRED_VARS cfitsio_INCLUDE_DIR cfitsio_LIBRARY VERSION_VAR cfitsio_VERSION_STRING)
set(cfitsio_FOUND ${CFITSIO_FOUND})
