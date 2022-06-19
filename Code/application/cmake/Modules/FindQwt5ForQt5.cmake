#message(STATUS "Qt5Gui_INCLUDE_DIRS: ${Qt5Gui_INCLUDE_DIRS}")
find_path(Qt5_INCLUDE_DIR QtGui HINTS ${Qt5Gui_INCLUDE_DIRS})
#message(STATUS "Qt5_INCLUDE_DIR: ${Qt5_INCLUDE_DIR}")

find_path(Qwt5ForQt5_INCLUDE_DIR qwt.h HINTS ${Qt5_INCLUDE_DIR} PATH_SUFFIXES qwt5-qt5 qt5/qwt5-qt5 qt5/qwt5 qwt5 qt5/qwt qwt)

message(STATUS "Qwt5ForQt5_INCLUDE_DIR: ${Qwt5ForQt5_INCLUDE_DIR}")

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

if(NOT QT5_LIBRARY_DIR)
   find_path(QT5_LIBRARY_DIR NAMES libQt5Gui.so libQt5Gui.so.5 PATHS ${Qt5_DIR}/../.. ${Qt5_DIR}/.. ${Qt5_DIR})
endif()

if(EXISTS "${Qwt5ForQt5_LIBRARY}")
   set(Qwt5ForQt5_LIBRARY_RELEASE "${Qwt5ForQt5_LIBRARY}")
endif()
find_library(Qwt5ForQt5_LIBRARY_RELEASE NAMES qwt5-qt5  HINTS ${QT5_LIBRARY_DIR})
find_library(Qwt5ForQt5_LIBRARY_DEBUG   NAMES qwt5-qt5d HINTS ${QT5_LIBRARY_DIR})

include(SelectLibraryConfigurations)
select_library_configurations(Qwt5ForQt5) #sets Qwt5ForQt5_LIBRARY using Qwt5ForQt5_LIBRARY_DEBUG and Qwt5ForQt5_LIBRARY_RELEASE

set(Qwt5ForQt5_INCLUDE_DIRS ${Qwt5ForQt5_INCLUDE_DIR})
mark_as_advanced(Qwt5ForQt5_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qwt5ForQt5 FOUND_VAR Qwt5ForQt5_FOUND REQUIRED_VARS Qwt5ForQt5_INCLUDE_DIR Qwt5ForQt5_LIBRARY VERSION_VAR Qwt5ForQt5_VERSION_STRING)
