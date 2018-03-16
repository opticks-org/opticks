find_path(pcre_INCLUDE_DIR pcre.h)
if(pcre_INCLUDE_DIR AND EXISTS "${pcre_INCLUDE_DIR}/pcre.h")
    file(STRINGS "${pcre_INCLUDE_DIR}/pcre.h" pcre_Parsed_Major_Version REGEX "^#define PCRE_MAJOR.*[0-9]+.*$")
    file(STRINGS "${pcre_INCLUDE_DIR}/pcre.h" pcre_Parsed_Minor_Version REGEX "^#define PCRE_MINOR.*[0-9]+.*$")

    string(REGEX REPLACE "^.*PCRE_MAJOR.*([0-9]+).*$" "\\1" pcre_VERSION_MAJOR "${pcre_Parsed_Major_Version}")
    string(REGEX REPLACE "^.*PCRE_MINOR.*([0-9]+).*$" "\\1" pcre_VERSION_MINOR "${pcre_Parsed_Minor_Version}")

    set(pcre_VERSION_STRING "${pcre_VERSION_MAJOR}.${pcre_VERSION_MINOR}")
    set(pcre_MAJOR_VERSION "${pcre_VERSION_MAJOR}")
    set(pcre_MINOR_VERSION "${pcre_VERSION_MINOR}")
endif()

find_library(pcre_LIBRARY_RELEASE NAMES pcre)
find_library(pcre_LIBRARY_DEBUG NAMES pcreD)

include(SelectLibraryConfigurations)
select_library_configurations(pcre)

set(pcre_INCLUDE_DIRS ${pcre_INCLUDE_DIR})
mark_as_advanced(pcre_INCLUDE_DIR)
mark_as_advanced(pcre_DEFINITIONS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pcre REQUIRED_VARS pcre_INCLUDE_DIR pcre_LIBRARY VERSION_VAR pcre_VERSION_STRING)
set(pcre_FOUND ${PCRE_FOUND})
