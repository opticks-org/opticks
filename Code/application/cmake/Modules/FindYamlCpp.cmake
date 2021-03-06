find_path(YamlCpp_INCLUDE_DIR yaml.h PATH_SUFFIXES yaml-cpp)

if(YamlCpp_INCLUDE_DIR-NOTFOUND)
   message(ERROR " could not find YamlCpp_INCLUDE_DIR: ${YamlCpp_INCLUDE_DIR}")
endif()

find_library(YamlCpp_LIBRARY_RELEASE NAMES yaml-cpp yaml)
find_library(YamlCpp_LIBRARY_DEBUG NAMES yaml-cppd)

mark_as_advanced(YamlCpp_INCLUDE_DIR)
set(YamlCpp_INCLUDE_DIRS ${YamlCpp_INCLUDE_DIR})

include(SelectLibraryConfigurations)
select_library_configurations(YamlCpp) #sets YamlCpp_LIBRARY using YamlCpp_LIBRARY_DEBUG and YamlCpp_LIBRARY_RELEASE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YamlCpp REQUIRED_VARS YamlCpp_INCLUDE_DIR YamlCpp_LIBRARY)
if(YAMLCPP_FOUND)
   message(STATUS "YamlCpp_INCLUDE_DIR: " ${YamlCpp_INCLUDE_DIR})
   message(STATUS "YamlCpp_LIBRARY:     " ${YamlCpp_LIBRARY})

   set(Yaml_h ${YamlCpp_INCLUDE_DIR}/yaml.h)
   if(NOT EXISTS ${Yaml_h})
      set(Yaml_h ${YamlCpp_INCLUDE_DIR}/yaml-cpp/yaml.h)
   endif()
   if(NOT EXISTS ${Yaml_h})
      message(ERROR " could not find YamlCpp header ${Yaml_h}")
   endif()

   set(Node_h ${YamlCpp_INCLUDE_DIR}/node.h)
   if(NOT EXISTS ${Node_h})
      set(Node_h ${YamlCpp_INCLUDE_DIR}/node/node.h)
   endif()
   if(NOT EXISTS ${Node_h})
      message(ERROR " could not find YamlCpp header ${Node_h}")
   endif()

   set(Parser_h ${YamlCpp_INCLUDE_DIR}/parser.h)
   if(NOT EXISTS ${Parser_h})
      set(Parser_h ${YamlCpp_INCLUDE_DIR}/yaml-cpp/parser.h)
   endif()
   if(NOT EXISTS ${Parser_h})
      message(ERROR " could not find YamlCpp header ${Parser_h}")
   endif()

  # message(STATUS "YamlCPP headers: ${Yaml_h} ${Parser_h} ${Node_h}")
  file(STRINGS ${Parser_h} Parser_GetNextDocument_Str REGEX "GetNextDocument")
  file(STRINGS ${Node_h}   Node_GetType_Str REGEX "GetType")

  # Integer XYZ_VERSION_NUMBER is typically (10000*(MAJOR_VERSION) + 100*(MINOR_VERSION) + (PATCH_VERSION))
  # dotted  XYZ_VERSION_STRING is "MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION"
  # We usually only need VERSION_NUMBER, so the preprocessor can do integer arithmetic to determine version
  # message("Parser_GetNextDocument: " ${Parser_GetNextDocument_Str})
  if(Parser_GetNextDocument_Str)
    if(Node_GetType_Str)
      set(YamlCpp_MAJOR_VERSION 0)
      set(YamlCpp_MINOR_VERSION 2)
      set(YamlCpp_PATCH_VERSION 5)
      set(YamlCpp_VERSION_NUMBER 205)
      set(YamlCpp_VERSION_STRING "0.2.5")
    else()
      set(YamlCpp_MAJOR_VERSION 0)
      set(YamlCpp_MINOR_VERSION 3)
      set(YamlCpp_PATCH_VERSION 0)
      set(YamlCpp_VERSION_NUMBER 300)
      set(YamlCpp_VERSION_STRING "0.3.0")
    endif()
  else()
    # yaml-cpp versions 0.6.0 and greater at least have a pkg-congif yaml-cpp.pc file, should we care to query it.
    set(YamlCpp_MAJOR_VERSION 0)
    set(YamlCpp_MINOR_VERSION 6)
    set(YamlCpp_PATCH_VERSION 0)
    set(YamlCpp_VERSION_NUMBER 600)
    set(YamlCpp_VERSION_STRING "0.6.0")
  endif()
  add_compile_options(-DYAMLCPP_VERSION_NUMBER=${YamlCpp_VERSION_NUMBER})
endif()

if(YAMLCPP_FOUND)
   message(STATUS "Found YamlCpp version ${YamlCpp_VERSION_STRING}")
endif()

set(YamlCpp_FOUND ${YAMLCPP_FOUND})

if(NOT ${YamlCpp_VERSION_NUMBER} EQUAL 205)
   string(REPLACE "/yaml-cpp" "" YamlCpp_INCLUDE_DIR "${YamlCpp_INCLUDE_DIR}")
   if(YamlCpp_INCLUDE_DIR STREQUAL "/usr/include")
      set(YamlCpp_INCLUDE_DIR "")
   endif()
   message(STATUS "truncating YamlCpp_INCLUDE_DIR to ${YamlCpp_INCLUDE_DIR}")
   mark_as_advanced(YamlCpp_INCLUDE_DIR)
   set(YamlCpp_INCLUDE_DIRS ${YamlCpp_INCLUDE_DIR})
endif()
