find_path(OpenCollada_INCLUDE_DIR opencollada/COLLADAFramework/COLLADAFW.h)
#message(STATUS "OpenCollada_INCLUDE_DIR: ${OpenCollada_INCLUDE_DIR}")

set(opencollada_lib_names
   OpenCOLLADABaseUtils
   OpenCOLLADAFramework
   OpenCOLLADASaxFrameworkLoader
   OpenCOLLADAStreamWriter
   buffer
   ftoa
   GeneratedSaxParser
   MathMLSolver
   UTF
   zlib)

foreach(lib_name ${opencollada_lib_names})
   string(TOUPPER ${lib_name} LIB_NAME)
   find_library(OpenCollada_${LIB_NAME}_LIBRARY_DEBUG ${lib_name}d PATH_SUFFIXES opencollada)
   find_library(OpenCollada_${LIB_NAME}_LIBRARY_RELEASE ${lib_name} PATH_SUFFIXES opencollada)
   set(OpenCollada_${LIB_NAME}_LIBRARY optimized ${OpenCollada_${LIB_NAME}_LIBRARY_RELEASE})
   list(APPEND OpenCollada_LIBRARIES ${OpenCollada_${LIB_NAME}_LIBRARY})
endforeach()
#message(STATUS "OpenCollada_LIBRARIES: ${OpenCollada_LIBRARIES}")

set(OpenCollada_INCLUDE_DIRS ${OpenCollada_INCLUDE_DIR})
mark_as_advanced(OpenCollada_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCollada REQUIRED_VARS OpenCollada_INCLUDE_DIR OpenCollada_LIBRARIES)
set(OpenCollada_FOUND ${OPENCOLLADA_FOUND})
