#######################################################################################
# Qt/Qwt boilerplate shared between Opticks and Spectral.
#
# The subsequent find_package(Qt4...) (below) fails with Spectral' default
# Dependencies/64/bin/qmake, because that qmake incorrectly -- for this purpose --
# sets QT_BINARY_DIR=/tmp/<something>/install/bin, which does not yet exist.
#
# For now, we just work around the Dependencies/64/bin/qmake QT_BINARY_DIR quirk by setting
# QT_BINARY_DIR explicitly to Dependencies/64/bin by default, then make find_program(moc) etc.
# calls here, that /usr/share/cmake/Modules/FindQt4.cmake would otherwise call later after
# querying qmake for a possibly invalid QT_BINARY_DIR path:
set(HAVE_QT5 0)
set(HAVE_QSAVEFILE 0)  # Experimental, likely not an issue for Spectral
if(EXISTS ${DEPENDENCY_PATH}/bin/qmake)
   set(QT_BINARY_DIR "${DEPENDENCY_PATH}/bin" CACHE PATH "directory containing qmake, moc, uic, rcc. default ${DEPENDENCY_PATH}/bin")
else()
   # /usr/lib64/qt4/bin is an okay default for RedHat; we'll then make a better OS-dependent guess with CMAKE_SYSTEM_LIBRARY_PATH.
   # But we set a default first, in case the foreach() loop fails to find anything, and RedHat's /usr/lib64/qt4/bin is as good as any.
   set(QT_BINARY_DIR "/usr/lib64/qt4/bin" CACHE PATH "directory containing qmake, moc, uic, rcc. default /usr/lib64/qt4/bin")
   foreach(lib ${CMAKE_SYSTEM_LIBRARY_PATH})
      if(EXISTS "${lib}/qt4/bin")
         set(QT_BINARY_DIR "${lib}/qt4/bin" CACHE PATH "directory containing qmake, moc, uic, rcc. default ${lib}/qt4/bin")
         break()
      endif()
   endforeach()
endif()
message(STATUS "QT Binary Dir: ${QT_BINARY_DIR}")
set(QT_QMAKE_EXECUTABLE ${QT_BINARY_DIR}/qmake CACHE FILEPATH "path to qmake. default ${QT_BINARY_DIR}/qmake")
if(EXISTS ${QT_QMAKE_EXECUTABLE})
   message(STATUS "Found QT QMake Executable: ${QT_QMAKE_EXECUTABLE}")
endif()
if(EXISTS ${QT_QMAKE_EXECUTABLE})
   # Get Qt version from Qmake
   execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_VERSION OUTPUT_VARIABLE qt_version  OUTPUT_STRIP_TRAILING_WHITESPACE)
   execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS OUTPUT_VARIABLE qt_install_libs  OUTPUT_STRIP_TRAILING_WHITESPACE)
   message(STATUS "Found Qt version \"${qt_version}\"")
   if("${qt_version}" VERSION_GREATER "5.0.0")
      set(HAVE_QT5 1)
      set(HAVE_QSAVEFILE 1) # Experimental: Qt5 writes disk files more reliably with QSaveFile than with QFile
      set(QT_Qt5_DIR "${qt_install_libs}/cmake/Qt5" CACHE PATH "Path to directory containing Qt5Config.cmake")
      set(Qt5_DIR ${QT_Qt5_DIR})
      if(IS_DIRECTORY ${Qt5_DIR})
         message(STATUS "Found Qt5_DIR: ${Qt5_DIR}")
      else()
         message(WARNING  " ${Qt5_DIR} NOT FOUND")
      endif()
      # TODO: Determine if CMAKE_AUTOMOC etc may be used instead of qt*_wrap_cpp() etc. macros.
      #       Might simplify things a bit if they can.
      #       See https://cmake.org/cmake/help/v3.12/prop_tgt/AUTOMOC.html#prop_tgt:AUTOMOC
      # set(CMAKE_AUTOMOC ON)
      # set(CMAKE_AUTORCC ON)
      # set(CMAKE_AUTOUIC ON)
   else()
      # Qt4
      if(EXISTS ${QT_BINARY_DIR})
         find_program(QT_MOC_EXECUTABLE
            NAMES moc-qt4 moc4 moc
            PATHS ${QT_BINARY_DIR}
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
         find_program(QT_UIC_EXECUTABLE
            NAMES uic-qt4 uic4 uic
            PATHS ${QT_BINARY_DIR}
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
         find_program(QT_RCC_EXECUTABLE
            NAMES rcc
            PATHS ${QT_BINARY_DIR}
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
            )
         message(STATUS "Moc: ${QT_MOC_EXECUTABLE}")
         message(STATUS "Uic: ${QT_UIC_EXECUTABLE}")
         message(STATUS "Rcc: ${QT_RCC_EXECUTABLE}")
      else()
         message(WARNING " QT_BINARY_DIR ${QT_BINARY_DIR} does not exists")
      endif()
   endif()
else()
   message(WARNING " QT_QMAKE_EXECUTABLE ${QT_QMAKE_EXECUTABLE} does not exist")
endif()
add_definitions(-DHAVE_QT5=${HAVE_QT5})
add_definitions(-DHAVE_QSAVEFILE=${HAVE_QSAVEFILE})

if(HAVE_QT5)
   # We include(${QT_USE_FILE}) throughout for Qt4. Qt5 doesn't use it. Check if
   # QT_USE_FILE is empty, and set it to a dummy file if it is.
   if(NOT QT_USE_FILE)
      set(QT_USE_FILE QtDummy)
   endif()

   # Redefine some qt4 macros for qt5:
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
else()
   find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXml QtNetwork REQUIRED)
   set(Qt_VERSION_MAJOR 4)
   add_definitions(-DQt_VERSION_MAJOR=${Qt_VERSION_MAJOR})
   # Shouldn't need to set these, but try if there are problems:
   # set(QT_INCLUDE_DIRS ${QtCore_INCLUDE_DIRS};${QtGui_INCLUDE_DIRS};${QtOpenGL_INCLUDE_DIRS};${QtXml_INCLUDE_DIRS};${QtNetwork_INCLUDE_DIRS})
   # set(QT_LIBRARIES ${QtCore_LIBRARIES} ${QtGui_LIBRARIES} ${QtOpenGL_LIBRARIES} ${QtXml_LIBRARIES} ${QtNetwork_LIBRARIES})
endif(HAVE_QT5)
include_directories(${QT_INCLUDE_DIRS})
# Debug messages:
# Message(STATUS "QT_INCLUDES: ${QT_INCLUDES}")
# Message(STATUS "QT_INCLUDE_DIRS: ${QT_INCLUDE_DIRS}")
# Message(STATUS "QT_LIBRARIES: ${QT_LIBRARIES}")
#######################################################################################
if(HAVE_QT5)
   find_package(Qwt5ForQt5 REQUIRED)
   include_directories(${Qwt5ForQt5_INCLUDE_DIR})
   # It's important the user can verify which Qwt5-qt5 library she's linking,
   # in case she's specified a non-system qt5 and doesn't want to p;ivk up a
   # system qwt5-qt5 by mistake:
   if(Qwt5ForQt5_FOUND)
      message(STATUS "Found Qwt5ForQt5_LIBRARY: ${Qwt5ForQt5_LIBRARY}")
   else()
      message(STATUS "${Qwt5ForQt5_LIBRARY}")
   endif()
else()
   find_package(Qwt5ForQt4 REQUIRED)
   include_directories(${Qwt5ForQt4_INCLUDE_DIRS})
   if(Qwt5ForQt4_FOUND)
      message(STATUS "Found Qwt5ForQt4_LIBRARY: ${Qwt5ForQt4_LIBRARY}")
   else()
      message(STATUS "${Qwt5ForQt4_LIBRARY}")
   endif()
endif()
#######################################################################################
