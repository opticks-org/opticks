find_path(Gdal_INCLUDE_DIR gdal.h PATH_SUFFIXES gdal)
if(Gdal_INCLUDE_DIR AND EXISTS "${Gdal_INCLUDE_DIR}/gdal_version.h")
    file(STRINGS "${Gdal_INCLUDE_DIR}/gdal_version.h" Gdal_Parsed_Major_Version REGEX "^#.+define GDAL_VERSION_MAJOR +[0-9]+.*$")
    file(STRINGS "${Gdal_INCLUDE_DIR}/gdal_version.h" Gdal_Parsed_Minor_Version REGEX "^#.+define GDAL_VERSION_MINOR +[0-9]+.*$")
    file(STRINGS "${Gdal_INCLUDE_DIR}/gdal_version.h" Gdal_Parsed_Revision_Version REGEX "^#.+define GDAL_VERSION_REV +[0-9]+.*$")
    file(STRINGS "${Gdal_INCLUDE_DIR}/gdal_version.h" Gdal_Parsed_Build_Version REGEX "^#.+define GDAL_VERSION_BUILD +[0-9]+.*$")

    string(REGEX REPLACE "^.*GDAL_VERSION_MAJOR +([0-9]+).*$" "\\1" Gdal_VERSION_MAJOR "${Gdal_Parsed_Major_Version}")
    string(REGEX REPLACE "^.*GDAL_VERSION_MINOR +([0-9]+).*$" "\\1" Gdal_VERSION_MINOR "${Gdal_Parsed_Minor_Version}")
    string(REGEX REPLACE "^.*GDAL_VERSION_REV +([0-9]+).*$" "\\1" Gdal_VERSION_REVISION "${Gdal_Parsed_Revision_Version}")
    string(REGEX REPLACE "^.*GDAL_VERSION_BUILD +([0-9]+).*$" "\\1" Gdal_VERSION_BUILD "${Gdal_Parsed_Build_Version}")

    set(Gdal_VERSION_STRING "${Gdal_VERSION_MAJOR}.${Gdal_VERSION_MINOR}.${Gdal_VERSION_REVISION}")
    set(Gdal_MAJOR_VERSION "${Gdal_VERSION_MAJOR}")
    set(Gdal_MINOR_VERSION "${Gdal_VERSION_MINOR}")
    set(Gdal_PATCH_VERSION "${Gdal_VERSION_REVISION}")
endif()

find_library(Gdal_LIBRARY_RELEASE NAMES gdal_i gdal gdal${Gdal_VERSION_MAJOR}.${Gdal_VERSION_MINOR}.${Gdal_VERSION_BUILD})
find_library(Gdal_LIBRARY_DEBUG NAMES gdal_id gdald)

include(SelectLibraryConfigurations)
select_library_configurations(Gdal)

set(Gdal_INCLUDE_DIRS ${Gdal_INCLUDE_DIR})
mark_as_advanced(Gdal_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gdal REQUIRED_VARS Gdal_INCLUDE_DIR Gdal_LIBRARY VERSION_VAR Gdal_VERSION_STRING)
set(Gdal_FOUND ${GDAL_FOUND})
