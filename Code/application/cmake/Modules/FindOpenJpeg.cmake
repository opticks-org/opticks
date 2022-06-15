find_path(OpenJpeg_INCLUDE_DIR openjpeg.h PATH_SUFFIXES openjpeg-2.0 openjpeg-2.6 openjpeg-2.5 openjpeg-2.4 openjpeg-2.3 openjpeg)

find_library(OpenJpeg_LIBRARY_RELEASE NAMES openjp2 openjpeg)
find_library(OpenJpeg_LIBRARY_DEBUG NAMES openjp2d openjpegd)

include(SelectLibraryConfigurations)
select_library_configurations(OpenJpeg) #sets OpenJpeg_LIBRARY using OpenJpeg_LIBRARY_DEBUG and OpenJpeg_LIBRARY_RELEASE

set(OpenJpeg_INCLUDE_DIRS ${OpenJpeg_INCLUDE_DIR})
mark_as_advanced(OpenJpeg_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenJpeg REQUIRED_VARS OpenJpeg_INCLUDE_DIR OpenJpeg_LIBRARY)
set(OpenJpeg_FOUND ${OPENJPEG_FOUND})

if(OpenJpeg_FOUND)
   message(STATUS "OpenJpeg_LIBRARY: ${OpenJpeg_LIBRARY}")

   # shouldn't need the rest of this
   file(STRINGS "${OpenJpeg_INCLUDE_DIR}/openjpeg.h" OpenJpeg_opj_stream_seek_fn     REGEX "opj_stream_seek_fn")
   file(STRINGS "${OpenJpeg_INCLUDE_DIR}/openjpeg.h" OpenJpeg_opj_stream_seek_stream REGEX "opj_stream_seek_stream")
   if(NOT OpenJpeg_opj_stream_seek_fn)
      message(STATUS "OpenJpeg_opj_stream_seek_fn NOTFOUND")
   endif()
   if(NOT OpenJpeg_opj_stream_seek_stream)
      message(STATUS "OpenJpeg_opj_stream_seek_stream NOTFOUND")
   else()
      add_definitions(-DOPJ_STREAM_SEEK_STREAM_FOUND=1)
   endif()
endif()
