find_path(Glew_INCLUDE_DIR GL/glew.h)

find_library(Glew_LIBRARY_RELEASE NAMES glew32 GLEW)
find_library(Glew_LIBRARY_DEBUG NAMES glew32d GLEWd)

include(SelectLibraryConfigurations)
select_library_configurations(Glew) #sets Glew_LIBRARY using Glew_LIBRARY_DEBUG and Glew_LIBRARY_RELEASE

mark_as_advanced(Glew_INCLUDE_DIR)
set(Glew_INCLUDE_DIRS ${Glew_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Glew REQUIRED_VARS Glew_INCLUDE_DIR Glew_LIBRARY)

set(Glew_FOUND ${GLEW_FOUND})
