find_path(OpenCollada_INCLUDE_DIR COLLADAFW.h PATH_SUFFIXES opencollada)

set(opencollada_lib_names
   OpenCOLLADABaseUtils
   OpenCOLLADAFramework
   openCOLLADASaxFrameworkLoader
   OpenCOLLADAStreamWriter
   buffer
   ftoa
   GeneratedSaxParser
   MathMLSolver
   pcre
   UTF
   xml)

foreach(lib_name ${opencollada_lib_names})
   string(TOUPPER ${lib_name} LIB_NAME)
   find_library(OpenCollada_${LIB_NAME}_LIBRARY_DEBUG ${lib_name}d)
   find_library(OpenCollada_${LIB_NAME}_LIBRARY_RELEASE ${lib_name})
   set(OpenCollada_${LIB_NAME}_LIBRARY optimized ${OpenCollada_${LIB_NAME}_LIBRARY_RELEASE} debug ${OpenCollada_${LIB_NAME}_LIBRARY_DEBUG})
   list(APPEND OpenCollada_LIBRARIES ${OpenCollada_${LIB_NAME}_LIBRARY})
endforeach()

set(OpenCollada_INCLUDE_DIRS ${OpenCollada_INCLUDE_DIR})
mark_as_advanced(OpenCollada_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCollada REQUIRED_VARS OpenCollada_INCLUDE_DIR OpenCollada_LIBRARIES)
set(OpenCollada_FOUND ${OPENCOLLADA_FOUND})
