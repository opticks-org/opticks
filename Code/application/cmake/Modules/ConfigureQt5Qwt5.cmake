#######################################################################################
# Qt5/Qwt5 boilerplate shared between Opticks and Spectral.
#
if(NOT IS_DIRECTORY "${Qt5_DIR}")
   if(IS_DIRECTORY "/usr/lib64/cmake/Qt5")
      message(STATUS "Qt5_DIR ${Qt5_DIR} NOT FOUND, using /usr/lib64/cmake/Qt5")
      set(Qt5_DIR "/usr/lib64/cmake/Qt5")
   else()
      message(WARNING "Qt5_DIR ${Qt5_DIR} NOT FOUND, please set Qt5_DIR to the directory containing Qt5Config.cmake")
   endif()
endif()
if(IS_DIRECTORY ${Qt5_DIR})
   message(STATUS "Found Qt5_DIR: ${Qt5_DIR}")
else()
   message(WARNING  "Qt5_DIR ${Qt5_DIR} NOT FOUND")
endif()
find_package(Qt5 REQUIRED COMPONENTS Core Concurrent Gui Widgets OpenGL Xml Network PrintSupport PATHS ${Qt5_DIR})
message(STATUS "Qt5_FOUND: ${Qt5_FOUND}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(HAVE_QSAVEFILE 1)  # Experimental QSaveFile usage with journal and log files, likely not an issue for Spectral.

add_definitions(-DHAVE_QSAVEFILE=${HAVE_QSAVEFILE})

set(Qt_VERSION_MAJOR ${Qt5_VERSION_MAJOR})
add_definitions(-DQt_VERSION_MAJOR=${Qt5_VERSION_MAJOR})

set(QT_INCLUDE_DIRS ${Qt5Concurrent_INCLUDE_DIRS};${Qt5Core_INCLUDE_DIRS};${Qt5Gui_INCLUDE_DIRS};${Qt5Widgets_INCLUDE_DIRS};${Qt5OpenGL_INCLUDE_DIRS};${Qt5Xml_INCLUDE_DIRS};${Qt5Network_INCLUDE_DIRS};${Qt5PrintSupport_INCLUDE_DIRS})

set(QT_LIBRARIES Qt5::Concurrent Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Xml Qt5::Network Qt5::PrintSupport)

if (Qt5Gui_FOUND)
   set(QT5_INCLUDE_DIRS ${QT_INCLUDE_DIRS})
   # message(STATUS "Qt5Gui_INCLUDE_DIRS: ${Qt5Gui_INCLUDE_DIRS}")
   # message(STATUS "QT5_INCLUDE_DIRS: ${QT_INCLUDE_DIRS}")
   if(NOT QT5_LIBRARY_DIR)
      find_path(QT5_LIBRARY_DIR NAMES libQt5Gui.so libQt5Gui.so.5 PATHS ${Qt5_DIR}/../.. ${Qt5_DIR}/.. ${Qt5_DIR})
   endif()
   message(STATUS "QT5_LIBRARY_DIR: ${QT5_LIBRARY_DIR}")
endif()

include_directories(${QT_INCLUDE_DIRS})
# Debug messages:
# Message(STATUS "QT_INCLUDES: ${QT_INCLUDES}")
# Message(STATUS "QT_INCLUDE_DIRS: ${QT_INCLUDE_DIRS}")
Message(STATUS "QT_LIBRARIES: ${QT_LIBRARIES}")
#######################################################################################
find_package(Qwt5ForQt5 REQUIRED)
include_directories(${Qwt5ForQt5_INCLUDE_DIR})
message(STATUS "Qt5_INCLUDE_DIR: ${Qt5_INCLUDE_DIR}")
message(STATUS "Qwt5ForQt5_INCLUDE_DIR: ${Qwt5ForQt5_INCLUDE_DIR}")
# It's important the user can verify which Qwt5-qt5 library she's linking,
# in case she's specified a non-system qt5 and doesn't want to pick up a
# system Qwt5-qt5 by mistake:
if(Qwt5ForQt5_FOUND)
   message(STATUS "Qwt5ForQt5_LIBRARY: ${Qwt5ForQt5_LIBRARY}")
else()
   message(STATUS "${Qwt5ForQt5_LIBRARY}")
endif()
#######################################################################################
