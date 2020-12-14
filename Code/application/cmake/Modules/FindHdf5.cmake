find_path(Hdf5_INCLUDE_DIR hdf5.h)
if(Hdf5_INCLUDE_DIR AND EXISTS "${Hdf5_INCLUDE_DIR}/H5public.h")
    file(STRINGS "${Hdf5_INCLUDE_DIR}/H5public.h" Hdf5_Parsed_Major_Version REGEX "^#define H5_VERS_MAJOR.*[0-9]+.*$")
    file(STRINGS "${Hdf5_INCLUDE_DIR}/H5public.h" Hdf5_Parsed_Minor_Version REGEX "^#define H5_VERS_MINOR.*[0-9]+.*$")
    file(STRINGS "${Hdf5_INCLUDE_DIR}/H5public.h" Hdf5_Parsed_Release_Version REGEX "^#define H5_VERS_RELEASE.*[0-9]+.*$")

    string(REGEX REPLACE "^.*H5_VERS_MAJOR.*([0-9]+).*$" "\\1" Hdf5_VERSION_MAJOR "${Hdf5_Parsed_Major_Version}")
    string(REGEX REPLACE "^.*H5_VERS_MINOR.*([0-9]+).*$" "\\1" Hdf5_VERSION_MINOR "${Hdf5_Parsed_Minor_Version}")
    string(REGEX REPLACE "^.*H5_VERS_RELEASE.*([0-9]+).*$" "\\1" Hdf5_VERSION_RELEASE "${Hdf5_Parsed_Release_Version}")

    set(Hdf5_VERSION_STRING "${Hdf5_VERSION_MAJOR}.${Hdf5_VERSION_MINOR}.${Hdf5_VERSION_RELEASE}")
    set(Hdf5_MAJOR_VERSION "${Hdf5_VERSION_MAJOR}")
    set(Hdf5_MINOR_VERSION "${Hdf5_VERSION_MINOR}")
    set(Hdf5_PATCH_VERSION "${Hdf5_VERSION_RELEASE}")
endif()

find_library(Hdf5_LIBRARY_RELEASE NAMES hdf5dll hdf5)
find_library(Hdf5_LIBRARY_DEBUG NAMES hdf5ddll_D hdf5_D)

include(SelectLibraryConfigurations)
select_library_configurations(Hdf5)

set(Hdf5_DEFINITIONS "-D_HDF5USEDLL_ -DH5_BUILT_AS_DYNAMIC_LIB" CACHE STRING "HDF5 definitions")
set(Hdf5_INCLUDE_DIRS ${Hdf5_INCLUDE_DIR})
mark_as_advanced(Hdf5_INCLUDE_DIR)
mark_as_advanced(Hdf5_DEFINITIONS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Hdf5 REQUIRED_VARS Hdf5_INCLUDE_DIR Hdf5_LIBRARY VERSION_VAR Hdf5_VERSION_STRING)
set(Hdf5_FOUND ${HDF5_FOUND})
