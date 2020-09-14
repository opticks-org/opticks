find_path(Qwt5ForQt5_INCLUDE_DIR qwt.h PATH_SUFFIXES qwt5-qt5 qt5/qwt5-qt5)
if(Qwt5ForQt5_INCLUDE_DIR AND EXISTS "${Qwt5ForQt5_INCLUDE_DIR}/qwt_global.h")
    file(STRINGS "${Qwt5ForQt5_INCLUDE_DIR}/qwt_global.h" Qwt5ForQt5_Parsed_Version REGEX "^#define QWT_VERSION_STR +\"[^\"]+\"$")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"([0-9]+).*$" "\\1" Qwt5ForQt5_VERSION_MAJOR "${Qwt5ForQt5_Parsed_Version}")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"[0-9]+\\.([0-9]+).*$" "\\1" Qwt5ForQt5_VERSION_MINOR "${Qwt5ForQt5_Parsed_Version}")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" Qwt5ForQt5_VERSION_PATCH "${Qwt5ForQt5_Parsed_Version}")

    set(Qwt5ForQt5_VERSION_STRING "${Qwt5ForQt5_VERSION_MAJOR}.${Qwt5ForQt5_VERSION_MINOR}.${Qwt5ForQt5_VERSION_PATCH}")
    set(Qwt5ForQt5_MAJOR_VERSION "${Qwt5ForQt5_VERSION_MAJOR}")
    set(Qwt5ForQt5_MINOR_VERSION "${Qwt5ForQt5_VERSION_MINOR}")
    set(Qwt5ForQt5_PATCH_VERSION "${Qwt5ForQt5_VERSION_PATCH}")
endif()

find_library(Qwt5ForQt5_LIBRARY_RELEASE NAMES qwt5-qt5 PATH_SUFFIXES qwt5)
find_library(Qwt5ForQt5_LIBRARY_DEBUG   NAMES qwt5-qt5d PATH_SUFFIXES qwt5)

include(SelectLibraryConfigurations)
select_library_configurations(Qwt5ForQt5) #sets Qwt5ForQt5_LIBRARY using Qwt5ForQt5_LIBRARY_DEBUG and Qwt5ForQt5_LIBRARY_RELEASE
if(NOT Qwt5ForQt5_LIBRARY)
   set(Qwt5ForQt5_LIBRARY optimized ${Qwt5ForQt5_LIBRARY_RELEASE} debug ${Qwt5ForQt5_LIBRARY_DEBUG})
   set(Qwt5ForQt5_LIBRARY ${Qwt5ForQt5_LIBRARY} CACHE FILEPATH "The Qwt5ForQt5 library")
endif()

set(Qwt5ForQt5_INCLUDE_DIRS ${Qwt5ForQt5_INCLUDE_DIR})
mark_as_advanced(Qwt5ForQt5_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qwt5ForQt5 REQUIRED_VARS Qwt5ForQt5_INCLUDE_DIR Qwt5ForQt5_LIBRARY VERSION_VAR Qwt5ForQt5_VERSION_STRING)
set(Qwt5ForQt5_FOUND ${QWT5FORQT5_FOUND})

