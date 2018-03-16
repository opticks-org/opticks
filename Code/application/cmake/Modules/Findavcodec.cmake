find_path(avcodec_INCLUDE_DIR avcodec.h PATH_SUFFIXES ffmpeg libavcodec)
if(avcodec_INCLUDE_DIR AND EXISTS "${avcodec_INCLUDE_DIR}/avcodec.h")
    file(STRINGS "${avcodec_INCLUDE_DIR}/avcodec.h" avcodec_Parsed_Version REGEX "^#define LIBAVCODEC_VERSION +.+$")
    string(REGEX REPLACE "^.*LIBAVCODEC_VERSION +([0-9]+).*$" "\\1" avcodec_VERSION_MAJOR "${avcodec_Parsed_Version}")
    string(REGEX REPLACE "^.*LIBAVCODEC_VERSION +[0-9]+\\.([0-9]+).*$" "\\1" avcodec_VERSION_MINOR "${avcodec_Parsed_Version}")
    string(REGEX REPLACE "^.*LIBAVCODEC_VERSION +[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" avcodec_VERSION_PATCH "${avcodec_Parsed_Version}")

    set(avcodec_VERSION_STRING "${avcodec_VERSION_MAJOR}.${avcodec_VERSION_MINOR}.${avcodec_VERSION_PATCH}")
    set(avcodec_MAJOR_VERSION "${avcodec_VERSION_MAJOR}")
    set(avcodec_MINOR_VERSION "${avcodec_VERSION_MINOR}")
    set(avcodec_PATCH_VERSION "${avcodec_VERSION_PATCH}")
endif()

find_library(avcodec_LIBRARY_RELEASE NAMES avcodec)
find_library(avcodec_LIBRARY_DEBUG NAMES avcodecd)

include(SelectLibraryConfigurations)
select_library_configurations(avcodec)

set(avcodec_DEFINITIONS -DOFFSET_T_DEFINED)
if(WIN32)
    list(APPEND avcodec_DEFINITIONS -DEMULATE_INTTYPES -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_ISOC9X_SOURCE -DMSVC8 -DBUILD_SHARED_AV) 
endif()
set(avcodec_DEFINITIONS "${avcodec_DEFINITIONS}" CACHE STRING "avcodec definitions")

set(avcodec_INCLUDE_DIRS ${avcodec_INCLUDE_DIR})
mark_as_advanced(avcodec_INCLUDE_DIR)
mark_as_advanced(avcodec_DEFINITIONS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(avcodec REQUIRED_VARS avcodec_INCLUDE_DIR avcodec_LIBRARY VERSION_VAR avcodec_VERSION_STRING)
set(avcodec_FOUND ${AVCODEC_FOUND})
