find_path(OpenJpeg_INCLUDE_DIR openjpeg.h PATH_SUFFIXES openjpeg-2.0 openjpeg-2.4 openjpeg-2.3 openjpeg)

find_library(OpenJpeg_LIBRARY_RELEASE NAMES openjp2 openjpeg)
find_library(OpenJpeg_LIBRARY_DEBUG NAMES openjp2d openjpegd)

include(SelectLibraryConfigurations)
select_library_configurations(OpenJpeg) #sets OpenJpeg_LIBRARY using OpenJpeg_LIBRARY_DEBUG and OpenJpeg_LIBRARY_RELEASE

set(OpenJpeg_INCLUDE_DIRS ${OpenJpeg_INCLUDE_DIR})
mark_as_advanced(OpenJpeg_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenJpeg REQUIRED_VARS OpenJpeg_INCLUDE_DIR OpenJpeg_LIBRARY)
set(OpenJpeg_FOUND ${OPENJPEG_FOUND})
