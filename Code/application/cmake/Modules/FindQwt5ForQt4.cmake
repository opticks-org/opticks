find_path(Qwt5ForQt4_INCLUDE_DIR qwt.h PATH_SUFFIXES qwt-qt4 qwt5-qt4 qt4/qwt5-qt4 qt/qwt5-qt4)
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
#message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# If Opticks_DEPENDENCIES is set to it's default location, then
# Opticks_DEPENDENCIES will have been appended to CMAKE_PREFIX_PATH, and
# the qwt5 library we want from there is libqwt.so, specifically libqwt.so.5.2.1
# Problem arises if there is also a system library by the same unversioned name but
# but a different major version, e.g. /usr/lib64/libqwt.so might be linked to /usr/lib64/libqwt.so.6.1.4,
# which we certainly do not want. To avoid this problem, we look for first libqwt.so.5.2.1
# specifically by that version, and only if not found do we look for other versions under
# other names.
set(OLD_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
#list(PREPEND CMAKE_FIND_LIBRARY_SUFFIXES .so.5.2.1) # not quite specific enough
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so.5.2.1") # accept no substitutes!
find_library(Qwt5ForQt4_LIBRARY_RELEASE NAMES qwt) # will look only for libqwt.s0.5.2.1
find_library(Qwt5ForQt4_LIBRARY_DEBUG   NAMES qwtd)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${OLD_CMAKE_FIND_LIBRARY_SUFFIXES})
if(NOT Qwt5ForQt4_LIBRARY)
   find_library(Qwt5ForQt4_LIBRARY_RELEASE NAMES qwt5 qwt5-qt4 qwt-qt4)
   find_library(Qwt5ForQt4_LIBRARY_DEBUG   NAMES qwt5d qwt5-qt4d qwt-qt4d)
endif()

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

