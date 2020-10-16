CMake beta version build instructions
You will need cmake version 2.8 or newer.

Windows
-------
On Windows, it may be easier to use cmake-gui since you need to make a few changes to the configured cmake variables. These instructions use cmake-gui.

- Run Dependencies/retrieve-dependencies.bat
- In the 32 and 64 directories, copy all the Qt DLLs from lib to bin
- Add a qt.conf file to the bin directory (see below for the contents of this file)
- Run cmake-gui
- Set the source code directory to Code/application
- Set the build directory (for example Code/build)
- Hit Configure and select Visual Studio 10 (either regular or Win64 config, you'll need a separate build directory and cmake configuration for each)
- Once the configure completes, ensure there are no errors except the expected "* Boost (required version >= 1.45.0)" in the REQUIRED packages not found. (this indicates that the boost libs aren't found but opticks doesn't use them)
- Select "Grouped" and "Advanced" checkboxes
- Open the "Qwt5ForQt4" section. It will indicate that the debug lib isn't found. Change the value to the same as the release library except add a "debug" path. (i.e. c:/opticks/Dependencies/32/lib/qwt5.lib becomes c:/opticks/Dependencies/32/lib/debug/qwt5.lib)
- Do the same thing for the YamlCpp debug lib
- Hit Configure again. (and a third time if there are any red entries remaining)
- Hit Generate
- Open the Opticks.sln in the build folder
- Build debug or release as needed.

To run Opticks from the debugger, you'll need to right click on the Opticks project and make it the default startup project.

Linux
-----
- Run Dependencies/retrieve-dependencies.sh
- Create a qt.conf file in 64/bin (see below for the contents of this file)
- If you don't have libssl.so.0.9.8 installed (look in /usr/lib64, you might have libssl.so.1.0.0) then ln -s /usr/lib64/libssl.so.1.0.0 Dependences/lib/libssl.so.0.9.8
- Create a build dir (Code/build for example) and cd to that directory
- cmake /path/to/Code/application (on some systems such as Redhat, you might need to use cmake28 as cmake is an older version)
-- You can specify a bunch of options, the most interesting are:
--- -D CMAKE_INSTALL_PREFIX:PATH=/path/to/install/location             see below for more information on the install directory
--- -D CMAKE_BUILD_TYPE=Release                                        defaults to Debug, unlike Windows, you need a separate build dir for Debug and Release
- Verify that there are no errors (the Boost required version >= 1.45.0 is expected as the boost libs are not found...Opticks does not use the libs)
- Run make     (you can do a -j42 or something for parallel builds)

You may need to set the LD_LIBRARY_PATH to Dependencies/64/lib before running Opticks

Build directory vs. install directory
-------------------------------------
There is an Install target (a project on Windows or "make install" on Linux) which preps an install tree. The standard build directory can be used as-is for debugging but the generated binaries contain rpath entries which need to be removed. Building the install target will do this and copy them to an install directory. (can be changed during cmake configuration and defaults to an "install" directory which is a sibling to the build directory) It will also copy the SupportFiles, DefaultSettings, and other items needed for an opticks install. If adds the SDK includes to an "include" directory and the public libs to a "lib" directory.

qt.conf contents
----------------
[Paths]
Headers = ../include/qt4
Libraries = ../lib
Binaries = ../bin 
Plugins = ../plugins
