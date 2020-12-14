# Dependencies
A number of external dependencies are requried and a complete list can be found [here](../Dependencies/vcpkg.list). It's best to use systems libraries
on Linux when they are available (via apt, yum, etc.) and build additional libraries on your system. Details are left as an exercise to the reader.
On Windows, it is suggested that you use vcpkg to install dependencies.

## Installing with vcpkg
It is suggested that you use the submodule in `Dependencies/vcpkg` as it may contain ports which are not yet merged into the main Microsoft repository.
The following commands in `cmd.exe` will build and install package dependencies.
```
C:\dev\opticks> git submodule update
Submodule path 'Dependencies/vcpkg': checked out '84303dc1800f72cbca1ccd68cd7051c77e6bea76'              <--- NOTE: the hash may be different

C:\dev\opticks> set PATH=C:\dev\opticks\Dependencies\vcpkg;%PATH%

C:\dev\opticks> set VCPKG_DEFAULT_TRIPLET=x64-windows

C:\dev\opticks> Dependencies\vcpkg\boostrap-vcpkg.bat
...     <--- Lots of build messages

Building vcpkg.exe... done.

C:\dev\opticks> vcpkg install @Dependencies\vcpkg.list
...     <--- Lots of build and install messages. This could take a while.

C:\dev\opticks>
```

This will install all of the required and optional dependencies in `Dependencies\vcpkg\installed`. You can now build Opticks with cmake.

# Building with cmake
You can configure Opticks from the command line with the following commands executed in the `Code` directory. You must specify the platform to build
64-bit as 32-bit Opticks is no longer supported. On Windows, you may also want to specify the generator name if you have multiple versions of
Visual Studio installed and the default version for cmake is not the correct version. This is the optional `-G`.

```
cmake -B Build -S application -A x64 -G "Visual Studio 15 2017"
```

If you are using vcpkg you should use the vcpkg toolchain integration.
```
cmake -B Build -S application -DCMAKE_TOOLCHAIN_FILE=C:\dev\opticks\Dependencies\vcpkg\scripts\buildsystems\vcpkg.cmake
```

If you use `cmake-gui` with `vcpkg` you specify the platform and toolchain in the initial configuration dialog. Set `x64` for the platform and
the path to the `vcpkg.cmake` after selecting "Specify toolchain file for cross-compiling."

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
