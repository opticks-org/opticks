#######################################################################################
# Qt5/Qwt6 boilerplate shared between Opticks and Spectral.
#
set(HAVE_QSAVEFILE 0)  # Experimental QSaveFile usage with journal and log files, likely not an issue for Spectral.
if(WIN32 AND EXISTS ${DEPENDENCY_PATH}/bin/qmake.exe)
    #set(QT_BINARY_DIR "${DEPENDENCY_PATH}/bin" CACHE PATH "directory containing qmake, moc, uic, rcc. default ${DEPENDENCY_PATH}/bin")
   set(QT_BINARY_DIR "${DEPENDENCY_PATH}/bin")
elseif(EXISTS ${DEPENDENCY_PATH}/bin/qmake)
   set(QT_BINARY_DIR "${DEPENDENCY_PATH}/bin" CACHE PATH "directory containing qmake, moc, uic, rcc. default ${DEPENDENCY_PATH}/bin")
else()
   # /usr/lib64/qt5/bin is an okay default for RedHat; we'll then make a better OS-dependent guess with CMAKE_SYSTEM_LIBRARY_PATH.
   # But we set a default first, in case the foreach() loop fails to find anything, and RedHat's /usr/lib64/qt4/bin is as good as any.
   set(QT_BINARY_DIR "/usr/lib64/qt5/bin" CACHE PATH "directory containing qmake, moc, uic, rcc. default /usr/lib64/qt5/bin")
   foreach(lib ${CMAKE_SYSTEM_LIBRARY_PATH})
      if(EXISTS "${lib}/qt5/bin")
         set(QT_BINARY_DIR "${lib}/qt5/bin" CACHE PATH "directory containing qmake, moc, uic, rcc. default ${lib}/qt5/bin")
         break()
      endif()
   endforeach()
endif()
message(STATUS "QT Binary Dir: ${QT_BINARY_DIR}")

if(WIN32)
    set(QT_QMAKE_EXECUTABLE ${QT_BINARY_DIR}/qmake.exe CACHE FILEPATH "path to qmake. default ${QT_BINARY_DIR}/qmake")
else()
    set(QT_QMAKE_EXECUTABLE ${QT_BINARY_DIR}/qmake CACHE FILEPATH "path to qmake. default ${QT_BINARY_DIR}/qmake")
endif()

if(EXISTS ${QT_QMAKE_EXECUTABLE})
   message(STATUS "Found QT QMake Executable: ${QT_QMAKE_EXECUTABLE}")
   # Get Qt version from Qmake
   execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_VERSION OUTPUT_VARIABLE qt_version  OUTPUT_STRIP_TRAILING_WHITESPACE)
   execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS OUTPUT_VARIABLE qt_install_libs  OUTPUT_STRIP_TRAILING_WHITESPACE)
   message(STATUS "Found Qt version \"${qt_version}\"")
   if("${qt_version}" VERSION_LESS "5.9.1")
      message(ERROR " Qt version must be 5.9.1 or greater")
   endif()
   set(HAVE_QSAVEFILE 1) # Experimental: Qt5 writes disk files more reliably with QSaveFile than with QFile
   set(QT_Qt5_DIR "${qt_install_libs}/cmake/Qt5" CACHE PATH "Path to directory containing Qt5Config.cmake")
   set(Qt5_DIR ${QT_Qt5_DIR})
   if(IS_DIRECTORY ${Qt5_DIR})
      message(STATUS "Found Qt5_DIR: ${Qt5_DIR}")
   else()
      message(WARNING  " ${Qt5_DIR} NOT X2FOUND")
   endif()
   # TODO: Determine if CMAKE_AUTOMOC etc may be used instead of qt5_wrap_cpp() etc. macros below.
   #       Might simplify things a bit if they can.
   #       See https://cmake.org/cmake/help/v3.12/prop_tgt/AUTOMOC.html#prop_tgt:AUTOMOC
   # set(CMAKE_INCLUDE_CURRENT_DIR ON)
   # set(CMAKE_AUTOMOC ON)
   # set(CMAKE_AUTORCC ON)
   # set(CMAKE_AUTOUIC ON)
else()
   message(WARNING " QT_QMAKE_EXECUTABLE ${QT_QMAKE_EXECUTABLE} does not exist")
endif()
add_definitions(-DHAVE_QSAVEFILE=${HAVE_QSAVEFILE})

# We include(${QT_USE_FILE}) throughout for Qt4. Qt5 doesn't use it. Check if
# QT_USE_FILE is empty, and set it to a dummy file if it is.
if(NOT QT_USE_FILE)
   set(QT_USE_FILE QtDummy)
endif()

# Redefine some qt4 macros for qt5. These are used extensively throughout our CMake tree, Perhaps
# they can be eliminated in favor of CMAKE_AUTOMOC, CMAKE_AUTORCC, CMAKE_AUTOUIC instead?
macro(qt4_wrap_cpp)
   qt5_wrap_cpp(${ARGV})
endmacro()
macro(qt4_wrap_ui)
   qt5_wrap_ui(${ARGV})
endmacro()
macro(qt4_add_resources)
   qt5_add_resources(${ARGV})
endmacro()

find_package(Qt5 COMPONENTS Core Concurrent Gui Widgets OpenGL Xml Network PrintSupport REQUIRED)
set(Qt_VERSION_MAJOR ${Qt5_VERSION_MAJOR})
add_definitions(-DQt_VERSION_MAJOR=${Qt5_VERSION_MAJOR})

set(QT_INCLUDE_DIRS ${Qt5Concurrent_INCLUDE_DIRS};${Qt5Core_INCLUDE_DIRS};${Qt5Gui_INCLUDE_DIRS};${Qt5Widgets_INCLUDE_DIRS};${Qt5OpenGL_INCLUDE_DIRS};${Qt5Xml_INCLUDE_DIRS};${Qt5Network_INCLUDE_DIRS};${Qt5PrintSupport_INCLUDE_DIRS})

set(QT_LIBRARIES ${Qt5Concurrent_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Xml_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5PrintSupport_LIBRARIES})
# message(STATUS "QT_INCLUDE_DIRS: ${QT_INCLUDE_DIRS}")
# message(STATUS "QT_LIBRARIES: ${QT_LIBRARIES}")

include_directories(${QT_INCLUDE_DIRS})
# Debug messages:
# Message(STATUS "QT_INCLUDES: ${QT_INCLUDES}")
# Message(STATUS "QT_INCLUDE_DIRS: ${QT_INCLUDE_DIRS}")
# Message(STATUS "QT_LIBRARIES: ${QT_LIBRARIES}")
#######################################################################################
find_package(Qwt6ForQt5 REQUIRED)
include_directories(${Qwt6ForQt5_INCLUDE_DIR})
# It's important the user can verify which Qwt6-qt5 library she's linking,
# in case she's specified a non-system qt5 and doesn't want to pick up a
# system qwt6-qt5 by mistake:
if(Qwt6ForQt5_FOUND)
   message(STATUS "Found Qwt6ForQt5_LIBRARY: ${Qwt6ForQt5_LIBRARY}")
else()
   message(STATUS "${Qwt6ForQt5_LIBRARY}")
endif()
#######################################################################################
