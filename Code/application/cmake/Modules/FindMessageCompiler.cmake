find_program(MessageCompiler_EXECUTABLE mc PATHS "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Bin" "C:/Program Files/Microsoft SDKs/Windows/v7.0A/Bin")

mark_as_advanced(MessageCompiler_EXECUTABLE)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MessageCompiler REQUIRED_VARS MessageCompiler_EXECUTABLE)

if(MESSAGECOMPILER_FOUND)
    set(MessageCompiler_FOUND ${MESSAGECOMPILER_FOUND})
endif()
