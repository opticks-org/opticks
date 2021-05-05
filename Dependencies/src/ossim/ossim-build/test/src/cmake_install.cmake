# Install script for directory: C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/test/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/ossim")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xossimx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/bin/Debug/ossim-batch-testd.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/bin/Release/ossim-batch-test.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/bin/MinSizeRel/ossim-batch-test.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/bin/RelWithDebInfo/ossim-batch-test.exe")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/base/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/elevation/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/gsoc/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/imaging/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/parallel/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/point_cloud/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/projection/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/support_data/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/util/cmake_install.cmake")
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/test/src/vec/cmake_install.cmake")

endif()

