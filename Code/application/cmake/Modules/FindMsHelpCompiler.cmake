if(WIN32)
    find_program(MsHelpCompiler_EXECUTABLE hhc
    "[HKEY_CURRENT_USER\\Software\\Microsoft\\HTML Help Workshop;InstallDir]"
    "$ENV{ProgramFiles}/HTML Help Workshop"
    "C:/Program Files/HTML Help Workshop" 
    )
endif(WIN32)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MsHelpCompiler REQUIRED_VARS MsHelpCompiler_EXECUTABLE)
set(MsHelpCompiler_FOUND ${MSHELPCOMPILER_FOUND})
