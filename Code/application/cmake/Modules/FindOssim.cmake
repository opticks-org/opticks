find_path(Ossim_INCLUDE_DIR ossimConfig.h HINTS ${Ossim_INCLUDE_DIR} PATH_SUFFIXES ossim)

if(Ossim_INCLUDE_DIR-NOTFOUND)
   message(ERROR " could not find Ossim_INCLUDE_DIR: ${Ossim_INCLUDE_DIR}")
endif()

# message(STATUS "Ossim_INCLUDE_DIR: " ${Ossim_INCLUDE_DIR})
set(ossimConfig_h ${Ossim_INCLUDE_DIR}/ossimConfig.h)
file(STRINGS "${ossimConfig_h}" OSSIM_VERSION_STRING REGEX "^#.*define OSSIM_VERSION +.*$")
if(NOT OSSIM_VERSION_STRING)
   set(ossimVersion_h ${Ossim_INCLUDE_DIR}/ossimVersion.h)
   # message(STATUS "ossimVersion_h: ${ossimVersion_h}")
   if(EXISTS "${ossimVersion_h}")
      file(STRINGS "${ossimVersion_h}" OSSIM_VERSION_STRING REGEX "^#.*define OSSIM_VERSION +.*$")
   endif()
endif()
if(OSSIM_VERSION_STRING)
   string(REGEX REPLACE "^.*OSSIM_VERSION +(.*)$" "\\1" OSSIM_VERSION_STRING "${OSSIM_VERSION_STRING}")
   message(STATUS "Found Ossim version ${OSSIM_VERSION_STRING} in ${ossimVersion_h}")
endif()

find_library(Ossim_LIBRARY_RELEASE NAMES ossim)
find_library(Ossim_LIBRARY_DEBUG NAMES ossimd)

include(SelectLibraryConfigurations)
select_library_configurations(Ossim)

string(REPLACE "/ossim" "" Ossim_INCLUDE_DIR "${Ossim_INCLUDE_DIR}")
if(Ossim_INCLUDE_DIR STREQUAL "/usr/include")
   set(Ossim_INCLUDE_DIR "")
endif()


set(Ossim_INCLUDE_DIRS ${Ossim_INCLUDE_DIR})
mark_as_advanced(Ossim_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ossim REQUIRED_VARS Ossim_INCLUDE_DIR Ossim_LIBRARY)

if(OSSIM_FOUND)
   message(STATUS "Ossim_INCLUDE_DIR: " ${Ossim_INCLUDE_DIR})
   message(STATUS "Ossim_LIBRARY:     " ${Ossim_LIBRARY})
   set(Ossim_LIBRARIES ${Ossim_LIBRARY})
endif()
set(Ossim_FOUND ${OSSIM_FOUND})
