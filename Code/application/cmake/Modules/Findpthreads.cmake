find_path(pthreads_INCLUDE_DIR pthread.h)
if(pthreads_INCLUDE_DIR AND EXISTS "${pthreads_INCLUDE_DIR}/pthread.h")
    file(STRINGS "${pthreads_INCLUDE_DIR}/pthread.h" pthreads_Parsed_Version REGEX "^#define PTW32_VERSION +.+$")
    string(REGEX REPLACE "^.*PTW32_VERSION ([0-9]+).*$" "\\1" pthreads_VERSION_MAJOR "${pthreads_Parsed_Version}")
    string(REGEX REPLACE "^.*PTW32_VERSION [0-9]+,([0-9]+).*$" "\\1" pthreads_VERSION_MINOR "${pthreads_Parsed_Version}")
    string(REGEX REPLACE "^.*PTW32_VERSION [0-9]+,[0-9]+,([0-9]+).*$" "\\1" pthreads_VERSION_PATCH "${pthreads_Parsed_Version}")

    set(pthreads_VERSION_STRING "${pthreads_VERSION_MAJOR}.${pthreads_VERSION_MINOR}.${pthreads_VERSION_PATCH}")
    set(pthreads_MAJOR_VERSION "${pthreads_VERSION_MAJOR}")
    set(pthreads_MINOR_VERSION "${pthreads_VERSION_MINOR}")
    set(pthreads_PATCH_VERSION "${pthreads_VERSION_PATCH}")
endif()

find_library(pthreads_LIBRARY_RELEASE NAMES pthreadVC${pthreads_MAJOR_VERSION})
find_library(pthreads_LIBRARY_DEBUG NAMES pthreadVC${pthreads_MAJOR_VERSION}d)

include(SelectLibraryConfigurations)
select_library_configurations(pthreads) #sets pthreads_LIBRARY using pthreads_LIBRARY_DEBUG and pthreads_LIBRARY_RELEASE

set(pthreads_INCLUDE_DIRS ${pthreads_INCLUDE_DIR})
mark_as_advanced(pthreads_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pthreads REQUIRED_VARS pthreads_INCLUDE_DIR pthreads_LIBRARY VERSION_VAR pthreads_VERSION_STRING)
set(pthreads_FOUND ${PTHREADS_FOUND})
