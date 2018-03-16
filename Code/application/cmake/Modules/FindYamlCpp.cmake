find_path(YamlCpp_INCLUDE_DIR yaml.h PATH_SUFFIXES yaml-cpp)

find_library(YamlCpp_LIBRARY_RELEASE NAMES yaml-cpp yaml)
find_library(YamlCpp_LIBRARY_DEBUG NAMES yaml-cppd)

include(SelectLibraryConfigurations)
select_library_configurations(YamlCpp) #sets YamlCpp_LIBRARY using YamlCpp_LIBRARY_DEBUG and YamlCpp_LIBRARY_RELEASE

mark_as_advanced(YamlCpp_INCLUDE_DIR)
set(YamlCpp_INCLUDE_DIRS ${YamlCpp_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YamlCpp REQUIRED_VARS YamlCpp_INCLUDE_DIR YamlCpp_LIBRARY)

set(YamlCpp_FOUND ${YAMLCPP_FOUND})
