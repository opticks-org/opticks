#######################################################################################
# Qt5/Qwt5 boilerplate shared between Opticks and Spectral.
#
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(HAVE_QSAVEFILE 1)  # Experimental QSaveFile usage with journal and log files, likely not an issue for Spectral.

add_definitions(-DHAVE_QSAVEFILE=${HAVE_QSAVEFILE})

find_package(Qt5 COMPONENTS Core Concurrent Gui Widgets OpenGL Xml Network PrintSupport REQUIRED)
set(Qt_VERSION_MAJOR ${Qt5_VERSION_MAJOR})
add_definitions(-DQt_VERSION_MAJOR=${Qt5_VERSION_MAJOR})

set(QT_INCLUDE_DIRS ${Qt5Concurrent_INCLUDE_DIRS};${Qt5Core_INCLUDE_DIRS};${Qt5Gui_INCLUDE_DIRS};${Qt5Widgets_INCLUDE_DIRS};${Qt5OpenGL_INCLUDE_DIRS};${Qt5Xml_INCLUDE_DIRS};${Qt5Network_INCLUDE_DIRS};${Qt5PrintSupport_INCLUDE_DIRS})

set(QT_LIBRARIES Qt5::Concurrent Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Xml Qt5::Network Qt5::PrintSupport)
#message(STATUS "QT_INCLUDE_DIRS: ${QT_INCLUDE_DIRS}")
#message(STATUS "QT_LIBRARIES: ${QT_LIBRARIES}")

include_directories(${QT_INCLUDE_DIRS})
# Debug messages:
# Message(STATUS "QT_INCLUDES: ${QT_INCLUDES}")
# Message(STATUS "QT_INCLUDE_DIRS: ${QT_INCLUDE_DIRS}")
# Message(STATUS "QT_LIBRARIES: ${QT_LIBRARIES}")
#######################################################################################
find_package(Qwt5ForQt5 REQUIRED)
include_directories(${Qwt5ForQt5_INCLUDE_DIR})
# It's important the user can verify which Qwt5-qt5 library she's linking,
# in case she's specified a non-system qt5 and doesn't want to pick up a
# system Qwt5-qt5 by mistake:
if(Qwt5ForQt5_FOUND)
   message(STATUS "Found Qwt5ForQt5_LIBRARY: ${Qwt5ForQt5_LIBRARY}")
else()
   message(STATUS "${Qwt5ForQt5_LIBRARY}")
endif()
#######################################################################################
