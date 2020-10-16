find_path(proj_INCLUDE_DIR proj_api.h)

if(proj_INCLUDE_DIR AND EXISTS "${proj_INCLUDE_DIR}/proj_api.h")
    file(STRINGS "${proj_INCLUDE_DIR}/proj_api.h" proj_Parsed_Version REGEX "^#define PJ_VERSION +[0-9]+.*$")
    string(SUBSTRING ${proj_Parsed_Version} 19 -1 proj_Parsed_Version)
    math(EXPR proj_MAJOR_VERSION "${proj_Parsed_Version} / 100")
    math(EXPR proj_MINOR_VERSION "${proj_Parsed_Version} % 100 / 10")
    math(EXPR proj_PATCH_VERSION "${proj_Parsed_Version} % 100 % 10")
    set(proj_VERSION_STRING "${proj_MAJOR_VERSION}.${proj_MINOR_VERSION}.${proj_PATCH_VERSION}")
endif()

find_library(proj_LIBRARY_RELEASE NAMES proj)
find_library(proj_LIBRARY_DEBUG NAMES projd)

include(SelectLibraryConfigurations)
select_library_configurations(proj) #sets proj_LIBRARY using proj_LIBRARY_DEBUG and proj_LIBRARY_RELEASE

mark_as_advanced(proj_INCLUDE_DIR)
set(proj_INCLUDE_DIRS ${proj_INCLUDE_DIR})

set(proj_LIBRARIES ${proj_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(proj REQUIRED_VARS proj_INCLUDE_DIR proj_LIBRARY VERSION_VAR proj_VERSION_STRING)

set(proj_FOUND ${GEOTIFF_FOUND})
