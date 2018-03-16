find_path(Cg_INCLUDE_DIR Cg/cg.h)

if(Cg_INCLUDE_DIR AND EXISTS "${Cg_INCLUDE_DIR}/Cg/cg.h")
    file(STRINGS "${Cg_INCLUDE_DIR}/Cg/cg.h" Cg_Parsed_Version REGEX "^#define CG_VERSION_NUM *[0-9]+ *$")

    math(EXPR Cg_MAJOR_VERSION "${Cg_Parsed_Version} / 1000")
    math(EXPR Cg_MINOR_VERSION "${Cg_Parsed_Version} % 1000 / 100")
    set(Cg_VERSION_STRING "${Cg_MAJOR_VERSION}.${Cg_MINOR_VERSION}")
endif()

find_library(Cg_LIBRARY_RELEASE NAMES Cg)

include(SelectLibraryConfigurations)
select_library_configurations(Cg) #sets Cg_LIBRARY using Cg_LIBRARY_DEBUG and Cg_LIBRARY_RELEASE

#TODO: use GL component for this detection
find_path(Cg_GL_INCLUDE_DIR Cg/cgGL.h)
find_library(Cg_GL_LIBRARY_RELEASE NAMES CgGL)
select_library_configurations(Cg_GL)

mark_as_advanced(Cg_INCLUDE_DIR Cg_GL_INCLUDE_DIR)
set(Cg_INCLUDE_DIRS ${Cg_INCLUDE_DIR} ${Cg_GL_INCLUDE_DIR}) #use components for Cg_GL stuff

set(Cg_LIBRARIES ${Cg_LIBRARY} ${Cg_GL_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cg REQUIRED_VARS Cg_INCLUDE_DIR Cg_LIBRARY VERSION_VAR Cg_VERSION_STRING)

set(Cg_FOUND ${CG_FOUND})
