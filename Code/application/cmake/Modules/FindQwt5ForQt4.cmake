find_path(Qwt5ForQt4_INCLUDE_DIR qwt.h PATH_SUFFIXES qwt-qt4)
if(Qwt5ForQt4_INCLUDE_DIR AND EXISTS "${Qwt5ForQt4_INCLUDE_DIR}/qwt_global.h")
    file(STRINGS "${Qwt5ForQt4_INCLUDE_DIR}/qwt_global.h" Qwt5ForQt4_Parsed_Version REGEX "^#define QWT_VERSION_STR +\"[^\"]+\"$")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"([0-9]+).*$" "\\1" Qwt5ForQt4_VERSION_MAJOR "${Qwt5ForQt4_Parsed_Version}")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"[0-9]+\\.([0-9]+).*$" "\\1" Qwt5ForQt4_VERSION_MINOR "${Qwt5ForQt4_Parsed_Version}")
    string(REGEX REPLACE "^.*QWT_VERSION_STR +\"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" Qwt5ForQt4_VERSION_PATCH "${Qwt5ForQt4_Parsed_Version}")

    set(Qwt5ForQt4_VERSION_STRING "${Qwt5ForQt4_VERSION_MAJOR}.${Qwt5ForQt4_VERSION_MINOR}.${Qwt5ForQt4_VERSION_PATCH}")
    set(Qwt5ForQt4_MAJOR_VERSION "${Qwt5ForQt4_VERSION_MAJOR}")
    set(Qwt5ForQt4_MINOR_VERSION "${Qwt5ForQt4_VERSION_MINOR}")
    set(Qwt5ForQt4_PATCH_VERSION "${Qwt5ForQt4_VERSION_PATCH}")
endif()

find_library(Qwt5ForQt4_LIBRARY_RELEASE NAMES qwt5 qwt-qt4 qwt)
find_library(Qwt5ForQt4_LIBRARY_DEBUG NAMES qwt5d qwt-qt4d qwt)

include(SelectLibraryConfigurations)
select_library_configurations(Qwt5ForQt4) #sets Qwt5ForQt4_LIBRARY using Qwt5ForQt4_LIBRARY_DEBUG and Qwt5ForQt4_LIBRARY_RELEASE
if(NOT Qwt5ForQt4_LIBRARY)
   set(Qwt5ForQt4_LIBRARY optimized ${Qwt5ForQt4_LIBRARY_RELEASE} debug ${Qwt5ForQt4_LIBRARY_DEBUG})
   set(Qwt5ForQt4_LIBRARY ${Qwt5ForQt4_LIBRARY} CACHE FILEPATH "The Qwt5ForQt4 library")
endif()

set(Qwt5ForQt4_INCLUDE_DIRS ${Qwt5ForQt4_INCLUDE_DIR})
mark_as_advanced(Qwt5ForQt4_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qwt5ForQt4 REQUIRED_VARS Qwt5ForQt4_INCLUDE_DIR Qwt5ForQt4_LIBRARY VERSION_VAR Qwt5ForQt4_VERSION_STRING)
set(Qwt5ForQt4_FOUND ${QWT5FORQT4_FOUND})

