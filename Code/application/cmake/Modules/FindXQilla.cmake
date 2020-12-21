find_path(XQilla_INCLUDE_DIR xqilla/xqilla-simple.hpp)

find_library(XQilla_LIBRARY_RELEASE NAMES xqilla23 xqilla)
find_library(XQilla_LIBRARY_DEBUG NAMES xqilla23d xqillad)

include(SelectLibraryConfigurations)
select_library_configurations(XQilla) #sets XQilla_LIBRARY using XQilla_LIBRARY_DEBUG and XQilla_LIBRARY_RELEASE

set(XQilla_INCLUDE_DIRS ${XQilla_INCLUDE_DIR})
mark_as_advanced(XQilla_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XQilla REQUIRED_VARS XQilla_INCLUDE_DIR XQilla_LIBRARY)
set(XQilla_FOUND ${XQILLA_FOUND})
