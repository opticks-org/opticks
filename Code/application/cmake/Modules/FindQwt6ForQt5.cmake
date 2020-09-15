find_path(Qwt6ForQt5_INCLUDE_DIR qwt.h PATH_SUFFIXES qwt qt5/qwt)
if(Qwt6ForQt5_INCLUDE_DIR AND EXISTS "${Qwt6ForQt5_INCLUDE_DIR}/qwt_global.h")
    file(STRINGS "${Qwt6ForQt5_INCLUDE_DIR}/qwt_global.h" Qwt6ForQt5_Parsed_Version REGEX "^#define QWT_VERSION_STR +\"[^\"]+\"$")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"([0-9]+).*$" "\\1" Qwt6ForQt5_VERSION_MAJOR "${Qwt6ForQt5_Parsed_Version}")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"[0-9]+\\.([0-9]+).*$" "\\1" Qwt6ForQt5_VERSION_MINOR "${Qwt6ForQt5_Parsed_Version}")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" Qwt6ForQt5_VERSION_PATCH "${Qwt6ForQt5_Parsed_Version}")

    set(Qwt6ForQt5_VERSION_STRING "${Qwt6ForQt5_VERSION_MAJOR}.${Qwt6ForQt5_VERSION_MINOR}.${Qwt6ForQt5_VERSION_PATCH}")
    set(Qwt6ForQt5_MAJOR_VERSION "${Qwt6ForQt5_VERSION_MAJOR}")
    set(Qwt6ForQt5_MINOR_VERSION "${Qwt6ForQt5_VERSION_MINOR}")
    set(Qwt6ForQt5_PATCH_VERSION "${Qwt6ForQt5_VERSION_PATCH}")
endif()

find_library(Qwt6ForQt5_LIBRARY_RELEASE NAMES qwt-qt5)
find_library(Qwt6ForQt5_LIBRARY_DEBUG   NAMES qwt-qt5)

include(SelectLibraryConfigurations)
select_library_configurations(Qwt6ForQt5) #sets Qwt6ForQt5_LIBRARY using Qwt6ForQt5_LIBRARY_DEBUG and Qwt6ForQt5_LIBRARY_RELEASE
if(NOT Qwt6ForQt5_LIBRARY)
   set(Qwt6ForQt5_LIBRARY optimized ${Qwt6ForQt5_LIBRARY_RELEASE} debug ${Qwt6ForQt5_LIBRARY_DEBUG})
   set(Qwt6ForQt5_LIBRARY ${Qwt6ForQt5_LIBRARY} CACHE FILEPATH "The Qwt6ForQt5 library")
endif()

set(Qwt6ForQt5_INCLUDE_DIRS ${Qwt6ForQt5_INCLUDE_DIR})
mark_as_advanced(Qwt6ForQt5_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qwt6ForQt5 REQUIRED_VARS Qwt6ForQt5_INCLUDE_DIR Qwt6ForQt5_LIBRARY VERSION_VAR Qwt6ForQt5_VERSION_STRING)
set(Qwt6ForQt5_FOUND ${QWT6FORQT5_FOUND})

