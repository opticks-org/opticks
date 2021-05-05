# Install script for directory: C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/dev/opticks-org/opticks/Dependencies/src/ossim-install")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ossim/projection" TYPE FILE FILES
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/projection/ossim_epsg_projections-v7_4.csv"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/projection/ossim_harn_state_plane_epsg.csv"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/projection/ossim_harn_state_plane_esri.csv"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/projection/ossim_state_plane_readme.txt"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/projection/ossim_state_plane_spcs.csv"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/projection/ossim_wkt_pcs.csv"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xossimx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ossim/util" TYPE FILE FILES
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/util/ossimChipProcUtil.json"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/util/ossimHlzUtil.json"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/util/ossimShorelineUtil.json"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/util/ossimSlopeUtil.json"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/util/ossimSlopeUtilX.json"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/util/ossimViewshedUtil.json"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xossimx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ossim/fonts" TYPE FILE FILES
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/fonts/arial.ttf"
    "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim/share/ossim/fonts/times.ttf"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/src/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/dev/opticks-org/opticks/Dependencies/src/ossim/ossim-build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
