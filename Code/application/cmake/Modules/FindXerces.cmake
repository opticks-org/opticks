find_path(Xerces_INCLUDE_DIR xercesc/util/XercesVersion.hpp)
if(Xerces_INCLUDE_DIR AND EXISTS "${Xerces_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp")
    file(STRINGS "${Xerces_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp" Xerces_Parsed_Major_Version REGEX "^#define XERCES_VERSION_MAJOR +[0-9]+.*$")
    file(STRINGS "${Xerces_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp" Xerces_Parsed_Minor_Version REGEX "^#define XERCES_VERSION_MINOR +[0-9]+.*$")
    file(STRINGS "${Xerces_INCLUDE_DIR}/xercesc/util/XercesVersion.hpp" Xerces_Parsed_Revision_Version REGEX "^#define XERCES_VERSION_REVISION +[0-9]+.*$")

    string(REGEX REPLACE "^.*XERCES_VERSION_MAJOR +([0-9]+).*$" "\\1" Xerces_VERSION_MAJOR "${Xerces_Parsed_Major_Version}")
    string(REGEX REPLACE "^.*XERCES_VERSION_MINOR +([0-9]+).*$" "\\1" Xerces_VERSION_MINOR "${Xerces_Parsed_Minor_Version}")
    string(REGEX REPLACE "^.*XERCES_VERSION_REVISION +([0-9]+).*$" "\\1" Xerces_VERSION_REVISION "${Xerces_Parsed_Revision_Version}")

    set(Xerces_VERSION_STRING "${Xerces_VERSION_MAJOR}.${Xerces_VERSION_MINOR}.${Xerces_VERSION_REVISION}")
    set(Xerces_MAJOR_VERSION "${Xerces_VERSION_MAJOR}")
    set(Xerces_MINOR_VERSION "${Xerces_VERSION_MINOR}")
    set(Xerces_PATCH_VERSION "${Xerces_VERSION_REVISION}")
endif()

find_library(Xerces_LIBRARY_RELEASE NAMES xerces-c_${Xerces_VERSION_MAJOR} xerces-c)
find_library(Xerces_LIBRARY_DEBUG NAMES xerces-c_${Xerces_VERSION_MAJOR}d xerces-cd)

include(SelectLibraryConfigurations)
select_library_configurations(Xerces) #sets Xerces_LIBRARY using Xerces_LIBRARY_DEBUG and Xerces_LIBRARY_RELEASE

set(Xerces_INCLUDE_DIRS ${Xerces_INCLUDE_DIR})
mark_as_advanced(Xerces_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xerces REQUIRED_VARS Xerces_INCLUDE_DIR Xerces_LIBRARY VERSION_VAR Xerces_VERSION_STRING)
set(Xerces_FOUND ${XERCES_FOUND})
