set(Opticks_VERSION "5.0.0Unofficial") # Change project(Opticks) below when VERSION becomes an official dotted-decimal
cmake_minimum_required (VERSION 2.8.12)

# Order is important on this. It MUST be before project()
if(DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build. options are: None, Debug, Release, RelWithDebInfo, MinSizeRel")
ELSE()
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build. options are: None, Debug, Release, RelWithDebInfo, MinSizeRel")
ENDIF()
if(CMAKE_INSTALL_PREFIX)
   set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Location to install Opticks" FORCE)
else()
   set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/../install CACHE PATH "Location to install Opticks" FORCE)
endif()

# Prevent in-source builds, build directory must be external to the source tree
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if(CMAKE_MAJOR_VERSION GREATER 2)
  cmake_policy(SET CMP0043 NEW)
  cmake_policy(SET CMP0048 NEW)
  cmake_policy(SET CMP0020 NEW)
endif()

if(CMAKE_MAJOR_VERSION GREATER 2)
   #project (Opticks VERSION ${Opticks_VERSION} DESCRIPTION "Ball Aerospace Opticks GIS") # VERSION must be dotted-decimal
   project (Opticks DESCRIPTION "Ball Aerospace Opticks GIS ${Opticks_VERSION}")
else()
   project (Opticks)
endif()

message(STATUS "Opticks Build Type: ${CMAKE_BUILD_TYPE}")
############################################################
# From https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
#
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/PlugIns;${CMAKE_INSTALL_PREFIX}/PlugIns/debug")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")
#################################################################

# Location of the Dependencies directory. On some platforms and configurations, it may not be necessary, but it's still searched.
if(NOT IS_DIRECTORY ${Opticks_DEPENDENCIES})
    set(Opticks_DEPENDENCIES $ENV{OPTICKSDEPENDENCIES} CACHE PATH "Location of Opticks pre-built dependencies")
endif()
Message(STATUS "Setting Opticks_DEPENDENCIES to ${Opticks_DEPENDENCIES}")
if(NOT IS_DIRECTORY "${Opticks_DEPENDENCIES}")
   message(STATUS "Opticks_DEPENDENCIES was not provided or does not exist, so required 3rd-party libraries will only be searched for on the system. "
      "Opticks_DEPENDENCIES can be retrieved by running the retrieve-dependencies script from the Dependencies directory.")
   set(Opticks_DEPENDENCIES "" CACHE PATH "Location of Opticks pre-built dependencies")
endif()

if(IS_DIRECTORY ${Opticks_DEPENDENCIES})
	# 64-bit or 32-bit dependencies?
	if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4)
	    set(OPTICKS_DEP_PATH ${Opticks_DEPENDENCIES}/32)
	elseif(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
	    set(OPTICKS_DEP_PATH ${Opticks_DEPENDENCIES}/64)
	endif()

	#set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${OPTICKS_DEP_PATH}")
	set(CMAKE_PREFIX_PATH "${OPTICKS_DEP_PATH}") # Spectral experience with TBB suggests this might make finding Dependencies packages more reliably against competing system packages.
endif()
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG) #-DDEBUG only for debug configurations
if(MSVC)
    add_definitions(-DNOMINMAX)
endif()
if(NOT Opticks_FLAGS_SET)
    set(Opticks_FLAGS_SET 1 CACHE INTERNAL "Compiler specific flags for Opticks")
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:except- /DHAVE_STRUCT_TIMESPEC /wd4996 /wd4267 /wd4250" CACHE STRING "Compiler flags" FORCE)
        set(Opticks_LINK_FLAGS /LARGEADDRESSAWARE)
        if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE" CACHE STRING "Compiler flags" FORCE)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI" CACHE STRING "Compiler flags" FORCE)
        elseif(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi" CACHE STRING "Compiler flags" FORCE)
        endif()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1 /Gy" CACHE STRING "Compiler flags" FORCE)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Zi /Oi /Ot /Oy /GF /GS-" CACHE STRING "Compiler flags" FORCE)

        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE" CACHE STRING "Linker flags" FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE" CACHE STRING "Linker flags" FORCE)
        
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug /OPT:NOREF /OPT:NOICF" CACHE STRING "Linker flags" FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /debug /OPT:NOREF /OPT:NOICF" CACHE STRING "Linker flags" FORCE)

        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib" CACHE STRING "Flags used by the linker during debug builds." FORCE)
        set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib" CACHE STRING "Flags used by the linker during debug builds." FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:msvcrt.lib" CACHE STRING "Flags used by the linker during debug builds." FORCE)
    endif()
    if(CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic" CACHE STRING "Compiler flags" FORCE)
    endif()
endif()

if(WIN32)
    set(ONE_OPTICKS_LIB FALSE)
    set(Opticks_WizardPrivate_LIBRARY Wizard)
else()
    set(ONE_OPTICKS_LIB TRUE)
    set(Opticks_WizardPrivate_LIBRARY OpticksLib)
endif()

set(Opticks_INCLUDE_DIRS
    "${Opticks_SOURCE_DIR}/Interfaces"
    "${Opticks_SOURCE_DIR}/PlugInLib"
    "${Opticks_SOURCE_DIR}/PlugInUtilities/pthreads-wrapper"
    "${Opticks_SOURCE_DIR}/PlugInUtilities/Interfaces"
    "${Opticks_SOURCE_DIR}/PlugInUtilities"
)
set(Opticks_HdfPlugInLib_INCLUDE_DIR "${Opticks_SOURCE_DIR}/HdfPlugInLib")
set(Opticks_NitfPlugInLib_INCLUDE_DIR "${Opticks_SOURCE_DIR}/NitfPlugInLib")
set(Opticks_SimpleApi_INCLUDE_DIR "${Opticks_SOURCE_DIR}/SimpleApiLib")

# Get the working copy revision
set(Opticks_Build_Revision "Unknown")
unset(OpticksGit)
find_package(Git)
if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD WORKING_DIRECTORY ${Opticks_SOURCE_DIR} OUTPUT_VARIABLE OpticksGit OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(OpticksGit)
        set(Opticks_Build_Revision "${OpticksGit}")
    endif()
endif()
Message(STATUS "Setting Build Revision To ${Opticks_Build_Revision}")
configure_file(cmake/Templates/BuildRevision.h "${Opticks_SOURCE_DIR}/Utilities")

# runtime configurations
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(WIN32)
    configure_file(cmake/Templates/Opticks.vcxproj.user "${CMAKE_BINARY_DIR}")
    configure_file(cmake/Templates/opticks.dep "${CMAKE_BINARY_DIR}/bin")
    configure_file(cmake/Templates/opticks-debug.dep "${CMAKE_BINARY_DIR}/bin")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    configure_file(cmake/Templates/opticks.dep "${CMAKE_BINARY_DIR}/bin")
else()
    configure_file(cmake/Templates/opticks-debug.dep "${CMAKE_BINARY_DIR}/bin/opticks.dep")
endif()
install(FILES cmake/Templates/opticks-debug-sdk.dep DESTINATION bin CONFIGURATIONS Debug)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/ApplicationUserSettings")

#----- Required packages -----
#add pthreads library on all platforms and verify it works at configure time
include(CheckCXXSourceCompiles)
if(WIN32)
    find_package(pthreads REQUIRED)
    set(CMAKE_REQUIRED_DEFINITIONS -DHAVE_STRUCT_TIMESPEC)
    set(CMAKE_REQUIRED_INCLUDES ${pthreads_INCLUDE_DIRS})
    include_directories(${CMAKE_REQUIRED_INCLUDES})
    set(CMAKE_REQUIRED_LIBRARIES ${pthreads_LIBRARIES})
    set(PTHREADS_LIBRARY ${CMAKE_REQUIRED_LIBRARIES})
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_REQUIRED_DEFINITIONS -pthread)
    add_definitions(${CMAKE_REQUIRED_DEFINITIONS})
    set(CMAKE_REQUIRED_LIBRARIES -pthread)
    set(PTHREADS_LIBRARY ${CMAKE_REQUIRED_LIBRARIES})
endif()

set(OpenGL_GL_PREFERENCE GLVND)

# Qt/Qwt configuration boilerplate shared with Spectral
if(IS_DIRECTORY ${OPTICKS_DEP_PATH})
	set(DEPENDENCY_PATH ${OPTICKS_DEP_PATH})
	include(ConfigureQt5Qwt5)
	unset(DEPENDENCY_PATH)
else()
	include(ConfigureQt5Qwt5)
endif()

find_package(Boost 1.45.0 REQUIRED COMPONENTS atomic)
find_package(Xerces REQUIRED)
find_package(XQilla REQUIRED)
find_package(Minizip REQUIRED)
find_package(YamlCpp REQUIRED)
find_package(Raptor REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Glew REQUIRED)
find_package(Ossim REQUIRED) #would like to make optional
#TODO: Make ossim finder fail if it's not the Ball version
find_package(Hdf4 COMPONENTS SD REQUIRED)
find_package(Hdf5 REQUIRED)
find_package(shapelib REQUIRED)
find_package(tiff REQUIRED)
find_package(geotiff REQUIRED)
find_package(proj REQUIRED)

#----- Optional packages -----
# ffmpeg packages for MovieExporter plugin
find_package(avcodec)
find_package(avformat)
find_package(avutil)
find_package(swscale)

find_package(cfitsio)
find_package(Gdal)
find_package(OpenCollada)
find_package(OpenCv COMPONENTS ImgProc)
find_package(OpenJpeg)
find_package(ehs)
find_package(pcre)
find_package(pme)

if(ehs_FOUND)
   include_directories(${ehs_INCLUDE_DIRS})
endif()

set(OLD_CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "${OPTICKS_DEP_PATH}/tools/graphviz")
find_package(Doxygen)
set(CMAKE_PREFIX_PATH "${OLD_CMAKE_PREFIX_PATH}")
find_package(MsHelpCompiler)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(EXPAT REQUIRED) #needed for raptor static library provided in pre-built dependencies
endif()

if(OpenCollada_FOUND)
   set(OPENCOLLADA_SUPPORT 1)
endif()
if(OpenJpeg_FOUND)
   set(JPEG2000_SUPPORT 1)
endif()
configure_file(cmake/Templates/DependencyConfigs.h .)
set(XERCES_NAMESPACE_VERSION "${Xerces_VERSION_MAJOR}_${Xerces_VERSION_MINOR}")
configure_file(cmake/Templates/XercesIncludes.h "${Opticks_SOURCE_DIR}/PlugInUtilities/Interfaces/")
add_subdirectory(ApiDocs)

#----- Adding public libraries -----
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/PlugIns")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/PlugIns/debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/PlugIns")
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(PlugInLib)
add_subdirectory(PlugInUtilities)
set(Opticks_LIBRARIES PlugInLib PlugInUtilities)
add_subdirectory(HdfPlugInLib)
set(Opticks_HdfPlugInLib_LIBRARY HdfPlugInLib)
add_subdirectory(NitfPlugInLib)
set(Opticks_NitfPlugInLib_LIBRARY NitfPlugInLib)
#----- Done adding public libraries -----
#----- Adding plug-ins -----
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/LibPrivate")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/LibPrivate")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/LibPrivate")

#for windows, .dll go into "PlugIns" and .lib go into "LibPrivate"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/PlugIns")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/PlugIns/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/PlugIns")
set(OLD_SHARED_LIBRARY_PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_subdirectory(ArcProxyLib)
add_subdirectory(PlugIns/src/AnnotationImagePalette)
add_subdirectory(PlugIns/src/Aspam)
add_subdirectory(PlugIns/src/AutoImporter)
add_subdirectory(PlugIns/src/BandBinning)
add_subdirectory(PlugIns/src/BandMath)
if(OpenCollada_FOUND)
   add_subdirectory(PlugIns/src/Collada)
endif()
if(OpenCv_FOUND)
   add_subdirectory(PlugIns/src/ConvolutionFilter)
endif()
add_subdirectory(PlugIns/src/CoreIo)
add_subdirectory(PlugIns/src/Covariance)
add_subdirectory(PlugIns/src/DataFusion)
add_subdirectory(PlugIns/src/Dted)
add_subdirectory(PlugIns/src/ENVI)
if(cfitsio_FOUND)
   add_subdirectory(PlugIns/src/Fits)
endif()
if(Gdal_FOUND)
   add_subdirectory(PlugIns/src/GdalImporter)
endif()
add_subdirectory(PlugIns/src/Generic)
add_subdirectory(PlugIns/src/GeographicFeatures)
add_subdirectory(PlugIns/src/GeoMosaic)
add_subdirectory(PlugIns/src/Georeference)
add_subdirectory(PlugIns/src/Hdf)
add_subdirectory(PlugIns/src/Ice)
add_subdirectory(PlugIns/src/ImageComparison)
if(Opticks_ServerUtilities_FOUND)
    add_subdirectory(PlugIns/src/Kml)
endif()
if(WIN32) 
    add_subdirectory(PlugIns/src/LegacyModule)
endif()
if(avcodec_FOUND AND avformat_FOUND AND avutil_FOUND)
   if(${avcodec_MAJOR_VERSION} EQUAL 51)
      # Opticks default 51.13.0 is likely the last ffmpeg avcodec to include img_convert(). Newer than that, must use sws_scale
      set(avcodec_USE_SWSCALE OFF CACHE BOOL "Set ON to use new sws_scale() for video image format conversion. Set OFF to use img_convert()")
   else()
      set(avcodec_USE_SWSCALE ON CACHE BOOL "Set ON to use sws_scale() for video image format conversion. Set OFF to use old img_convert(), if you have one.")
   endif()
   if(avcodec_USE_SWSCALE)
      add_definitions(-DUSE_SWSCALE=1)
   else()
      add_definitions(-DUSE_SWSCALE=0)
   endif()
   add_subdirectory(PlugIns/src/MovieExporter)
endif()
add_subdirectory(PlugIns/src/Nitf)
add_subdirectory(PlugIns/src/NitfCommonTre)
if(OpenCv_FOUND)
   add_subdirectory(PlugIns/src/ObjectFinding)
endif()
add_subdirectory(PlugIns/src/Pca)
add_subdirectory(PlugIns/src/Pictures)
add_subdirectory(PlugIns/src/PlugInSampler)
add_subdirectory(PlugIns/src/PlugInSamplerHdf)
add_subdirectory(PlugIns/src/PlugInSamplerQt)
add_subdirectory(PlugIns/src/Results)
add_subdirectory(PlugIns/src/Scripts)
add_subdirectory(PlugIns/src/SecondMoment)
add_subdirectory(PlugIns/src/ShapeFileExporter)
add_subdirectory(PlugIns/src/Sio)
add_subdirectory(PlugIns/src/Tutorial)
add_subdirectory(PlugIns/src/Wavelength)
add_subdirectory(PlugIns/src/WizardExecutor)
add_subdirectory(PlugIns/src/WizardItems)
if(Opticks_ServerUtilities_FOUND)
   if(!CMAKE_SYSTEM_NAME STREQUAL "Linux") # Currently broken on linux
      add_subdirectory(PlugIns/src/XmlRpc)
   endif()
endif()
set(CMAKE_SHARED_LIBRARY_PREFIX "${OLD_SHARED_LIBRARY_PREFIX}")

#----- Done adding plug-ins -----
#----- Adding private libraries and Opticks executables -----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")
add_subdirectory(SimpleApiLib)
set(Opticks_SimpleApi_LIBRARY SimpleApiLib)

include_directories(${Opticks_INCLUDE_DIRS})
include_directories(${Opticks_BINARY_DIR})
include_directories(
    ${Opticks_SOURCE_DIR}/Desktop
    ${Opticks_SOURCE_DIR}/Desktop/Layer
    ${Opticks_SOURCE_DIR}/Desktop/Animation
    ${Opticks_SOURCE_DIR}/Desktop/Plot
    ${Opticks_SOURCE_DIR}/Framework
    ${Opticks_SOURCE_DIR}/Gui
    ${Opticks_SOURCE_DIR}/Gui/GLView
    ${Opticks_SOURCE_DIR}/Gui/Graphic
    ${Opticks_SOURCE_DIR}/Gui/Image
    ${Opticks_SOURCE_DIR}/Gui/Undo
    ${Opticks_SOURCE_DIR}/Installer
    ${Opticks_SOURCE_DIR}/Model
    ${Opticks_SOURCE_DIR}/PlugInManager
    ${Opticks_SOURCE_DIR}/Utilities
    ${Opticks_SOURCE_DIR}/Wizard
)
include(${QT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Xerces_INCLUDE_DIRS})
include_directories(${XQilla_INCLUDE_DIRS})
include_directories(${Minizip_INCLUDE_DIRS})
include_directories(${YamlCpp_INCLUDE_DIRS})
include_directories(${Raptor_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${Glew_INCLUDE_DIRS})
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include_directories(${EXPAT_INCLUDE_DIRS})
endif()

add_definitions(-DAPPLICATION_XERCES)
set(OPTICKS_EXECUTABLE_LIBS
    Desktop
    Framework
    Gui
    Installer
    Model
    PlugInManager
    Utilities
    Wizard)
foreach(LIB_NAME IN LISTS OPTICKS_EXECUTABLE_LIBS)
    add_subdirectory(${LIB_NAME})
endforeach()

# CMake documentation cautions against file globing to set HEADER_FILES, SOURCE_FILES, etc.,
# as you could add source files to a large project, but CMake wouldn't regenerate as none of
# it's CMakeLists.txt had themselves changed. OTOH, maintaining these file lists is itself a
# bit of a task. I've chosen to go with CMake's recommendation for now.
set(INTERFACE_HEADER_FILES
    Interfaces/Animation.h
    Interfaces/AnimationController.h
    Interfaces/AnimationFrame.h
    Interfaces/AnimationServices.h
    Interfaces/AnimationToolBar.h
    Interfaces/AnnotationElement.h
    Interfaces/AnnotationLayer.h
    Interfaces/Any.h
    Interfaces/AnyData.h
    Interfaces/AoiElement.h
    Interfaces/AoiLayer.h
    Interfaces/AoiToolBar.h
    Interfaces/AppConfig.h
    Interfaces/ApplicationServices.h
    Interfaces/ArcObject.h
    Interfaces/Arrow.h
    Interfaces/ArrowObject.h
    Interfaces/Axis.h
    Interfaces/BadValues.h
    Interfaces/BitMask.h
    Interfaces/BitMaskObject.h
    Interfaces/Blob.h
    Interfaces/CartesianGridlines.h
    Interfaces/CartesianPlot.h
    Interfaces/CgmObject.h
    Interfaces/Classification.h
    Interfaces/ClassificationLayer.h
    Interfaces/ColorType.h
    Interfaces/ComplexData.h
    Interfaces/ConfigurationSettings.h
    Interfaces/ContextMenu.h
    Interfaces/ContextMenuAction.h
    Interfaces/ContextMenuActions.h
    Interfaces/Curve.h
    Interfaces/CurveCollection.h
    Interfaces/CustomLayer.h
    Interfaces/DataAccessor.h
    Interfaces/DataAccessorImpl.h
    Interfaces/DataDescriptor.h
    Interfaces/DataElement.h
    Interfaces/DataElementGroup.h
    Interfaces/DataRequest.h
    Interfaces/DataVariantFactory.h
    Interfaces/DateTime.h
    Interfaces/DesktopServices.h
    Interfaces/DimensionDescriptor.h
    Interfaces/DimensionObject.h
    Interfaces/DockWindow.h
    Interfaces/DynamicModule.h
    Interfaces/DynamicObject.h
    Interfaces/EastArrowObject.h
    Interfaces/EllipseObject.h
    Interfaces/EnumWrapper.h
    Interfaces/Executable.h
    Interfaces/ExecutableAgent.h
    Interfaces/ExportAgent.h
    Interfaces/Exporter.h
    Interfaces/External.h
    Interfaces/FileDescriptor.h
    Interfaces/FileFinder.h
    Interfaces/FileImageObject.h
    Interfaces/Filename.h
    Interfaces/Font.h
    Interfaces/FrameLabelObject.h
    Interfaces/GcpLayer.h
    Interfaces/GcpList.h
    Interfaces/Georeference.h
    Interfaces/GeoreferenceDescriptor.h
    Interfaces/GpuProgramDescriptor.h
    Interfaces/GraphicElement.h
    Interfaces/GraphicGroup.h
    Interfaces/GraphicLayer.h
    Interfaces/GraphicObject.h
    Interfaces/Gridlines.h
    Interfaces/Histogram.h
    Interfaces/HistogramPlot.h
    Interfaces/HistogramWindow.h
    Interfaces/ImageFilterDescriptor.h
    Interfaces/ImportAgent.h
    Interfaces/ImportDescriptor.h
    Interfaces/Importer.h
    Interfaces/Int64.h
    Interfaces/Interpreter.h
    Interfaces/InterpreterManager.h
    Interfaces/LatLonInsertObject.h
    Interfaces/LatLonLayer.h
    Interfaces/Layer.h
    Interfaces/LayerList.h
    Interfaces/LineObject.h
    Interfaces/Location.h
    Interfaces/LocationType.h
    Interfaces/Locator.h
    Interfaces/MeasurementLayer.h
    Interfaces/MenuBar.h
    Interfaces/MessageLog.h
    Interfaces/MessageLogMgr.h
    Interfaces/ModelServices.h
    Interfaces/Module.h
    Interfaces/ModuleManager.h
    Interfaces/MouseMode.h
    Interfaces/MultipointObject.h
    Interfaces/NorthArrowObject.h
    Interfaces/ObjectFactory.h
    Interfaces/Observer.h
    Interfaces/OpticksVersion.h
    Interfaces/Option.h
    Interfaces/OrthographicView.h
    Interfaces/PerspectiveView.h
    Interfaces/PlotGroup.h
    Interfaces/PlotObject.h
    Interfaces/PlotSet.h
    Interfaces/PlotSetGroup.h
    Interfaces/PlotView.h
    Interfaces/PlotWidget.h
    Interfaces/PlugIn.h
    Interfaces/PlugInArg.h
    Interfaces/PlugInArgList.h
    Interfaces/PlugInCallback.h
    Interfaces/PlugInDescriptor.h
    Interfaces/PlugInManagerServices.h
    Interfaces/PlugInRegistration.h
    Interfaces/PointCloudAccessor.h
    Interfaces/PointCloudAccessorImpl.h
    Interfaces/PointCloudDataDescriptor.h
    Interfaces/PointCloudDataRequest.h
    Interfaces/PointCloudElement.h
    Interfaces/PointCloudFileDescriptor.h
    Interfaces/PointCloudPager.h
    Interfaces/PointCloudView.h
    Interfaces/PointCloudWindow.h
    Interfaces/PointDataBlock.h
    Interfaces/Point.h
    Interfaces/PointSet.h
    Interfaces/PolarGridlines.h
    Interfaces/PolarPlot.h
    Interfaces/PolygonObject.h
    Interfaces/PolygonPlotObject.h
    Interfaces/PolylineObject.h
    Interfaces/ProductView.h
    Interfaces/ProductWindow.h
    Interfaces/Progress.h
    Interfaces/Properties.h
    Interfaces/PseudocolorLayer.h
    Interfaces/RasterDataDescriptor.h
    Interfaces/RasterElement.h
    Interfaces/RasterFileDescriptor.h
    Interfaces/RasterLayer.h
    Interfaces/RasterPage.h
    Interfaces/RasterPager.h
    Interfaces/RawImageObject.h
    Interfaces/RectangleObject.h
    Interfaces/RegionObject.h
    Interfaces/Resampler.h
    Interfaces/RoundedRectangleObject.h
    Interfaces/SafeSlot.h
    Interfaces/ScaleBarObject.h
    Interfaces/Serializable.h
    Interfaces/SessionExplorer.h
    Interfaces/SessionItem.h
    Interfaces/SessionItemDeserializer.h
    Interfaces/SessionItemSerializer.h
    Interfaces/SessionManager.h
    Interfaces/SettableSessionItem.h
    Interfaces/Signature.h
    Interfaces/SignatureDataDescriptor.h
    Interfaces/SignatureFileDescriptor.h
    Interfaces/SignatureLibrary.h
    Interfaces/SignaturePlot.h
    Interfaces/SignatureSet.h
    Interfaces/Slot.h
    Interfaces/SpatialDataView.h
    Interfaces/SpatialDataWindow.h
    Interfaces/SpecialMetadata.h
    Interfaces/Statistics.h
    Interfaces/Subject.h
    Interfaces/Testable.h
    Interfaces/Text.h
    Interfaces/TextObject.h
    Interfaces/ThresholdLayer.h
    Interfaces/TiePointLayer.h
    Interfaces/TiePointList.h
    Interfaces/ToolBar.h
    Interfaces/TrailObject.h
    Interfaces/TriangleObject.h
    Interfaces/TypeAwareObject.h
    Interfaces/TypesFile.h
    Interfaces/UInt64.h
    Interfaces/Units.h
    Interfaces/UtilityServices.h
    Interfaces/View.h
    Interfaces/ViewObject.h
    Interfaces/ViewWindow.h
    Interfaces/Wavelengths.h
    Interfaces/WidgetImageObject.h
    Interfaces/Window.h
    Interfaces/WizardItem.h
    Interfaces/WizardNode.h
    Interfaces/WizardObject.h
    Interfaces/WorkspaceWindow.h
)

set(SOURCE_FILES
    Main.cpp
    App.rc
)
if(WIN32)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/PlugInUtilities/EventMessages.rc PROPERTIES GENERATED 1)
    list(APPEND SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/PlugInUtilities/EventMessages.rc)
endif()
if(ONE_OPTICKS_LIB)
    foreach(LIB_NAME IN LISTS OPTICKS_EXECUTABLE_LIBS)
        get_directory_property(LIB_SOURCES DIRECTORY ${LIB_NAME} ALL_SOURCES)
        foreach(SOURCE IN LISTS LIB_SOURCES)
            list(APPEND OPTICKS_LIB_SOURCES "${LIB_NAME}/${SOURCE}")
        endforeach()
        get_directory_property(LIB_MOC_HEADERS DIRECTORY ${LIB_NAME} MOC_HEADERS)
        set(LIB_CONV_MOC_HEADERS "")
        foreach(SOURCE IN LISTS LIB_MOC_HEADERS)
            list(APPEND LIB_CONV_MOC_HEADERS "${LIB_NAME}/${SOURCE}")
        endforeach()
        qt5_wrap_cpp(LIB_MOC_OUTFILES ${LIB_CONV_MOC_HEADERS})
        list(APPEND OPTICKS_LIB_SOURCES ${LIB_MOC_OUTFILES} ${LIB_CONV_MOC_HEADERS})
        get_directory_property(LIB_QRC_FILES DIRECTORY ${LIB_NAME} QRC_FILES)
        set(LIB_CONV_QRC_FILES "")
        foreach(SOURCE IN LISTS LIB_QRC_FILES)
            list(APPEND LIB_CONV_QRC_FILES "${LIB_NAME}/${SOURCE}")
        endforeach()
        qt5_add_resources(LIB_RCC_OUTFILES ${LIB_CONV_QRC_FILES})
        list(APPEND OPTICKS_LIB_SOURCES ${LIB_RCC_OUTFILES} ${LIB_CONV_QRC_FILES})
    endforeach()
    add_library(OpticksLib STATIC ${OPTICKS_LIB_SOURCES})
    set(OPTICKS_EXECUTABLE_LIBS OpticksLib)
endif()
add_subdirectory(Batch)
add_executable(Opticks WIN32 ${SOURCE_FILES} ${INTERFACE_HEADER_FILES})
source_group(Interfaces FILES ${INTERFACE_HEADER_FILES})
target_link_libraries(Opticks
    ${OPTICKS_EXECUTABLE_LIBS}
    ${Opticks_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}    
    ${ZLIB_LIBRARIES}    
    ${YamlCpp_LIBRARIES}    
    ${Minizip_LIBRARIES}    
    ${Xerces_LIBRARIES}
    ${XQilla_LIBRARIES}
    ${Raptor_LIBRARIES}
    ${Glew_LIBRARIES}
    ${PTHREADS_LIBRARY}
    ${Qwt5ForQt5_LIBRARY}
    )

set_target_properties(Opticks PROPERTIES DEBUG_POSTFIX "d")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(Opticks PROPERTIES OUTPUT_NAME "linuxOpticks")
    target_link_libraries(Opticks
        ${EXPAT_LIBRARIES} -ldl # TODO Check if -ldl should go as -dl in set(CMAKE_REQUIRED_LIBRARIES -pthread) above
    )
endif()
if(WIN32)
    target_link_libraries(Opticks
        ${QT_QTMAIN_LIBRARY}
    )
endif()

if (EXISTS "CppTests" AND IS_DIRECTORY "CppTests")
    add_subdirectory(CppTests)
endif()
include(FeatureSummary)
feature_summary(WHAT ALL)

set(CODEDIR ${Opticks_SOURCE_DIR}/..)
if(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 4)
   set(BITS "32")
elseif(CMAKE_CXX_SIZEOF_DATA_PTR EQUAL 8)
   set(BITS "64")
endif()
install(TARGETS Opticks RUNTIME DESTINATION bin)

#mark variables as advanced that the finders should have marked
mark_as_advanced(FORCE QT_QMAKE_EXECUTABLE)
mark_as_advanced(FORCE Boost_LIB_DIAGNOSTIC_DEFINITIONS)

#----- Done adding private libraries and Opticks executables -----
#----- Adding install targets for other items -----
install(DIRECTORY ${Opticks_SOURCE_DIR}/../Release/DefaultSettings
   ${Opticks_SOURCE_DIR}/../Release/Extensions
   ${Opticks_SOURCE_DIR}/../Release/SupportFiles
   ${Opticks_SOURCE_DIR}/../Release/Templates  DESTINATION . FILES_MATCHING
   PATTERN "*" PATTERN ".svn" EXCLUDE PATTERN "_svn" EXCLUDE)
install(DIRECTORY ${Opticks_SOURCE_DIR}/Interfaces
   ${Opticks_SOURCE_DIR}/PlugInLib
   ${Opticks_SOURCE_DIR}/HdfPlugInLib ${Opticks_SOURCE_DIR}/NitfPlugInLib
   ${Opticks_SOURCE_DIR}/SimpleApiLib DESTINATION include FILES_MATCHING
   PATTERN "*.h" PATTERN ".svn" EXCLUDE PATTERN "_svn" EXCLUDE)
install(DIRECTORY ${Opticks_SOURCE_DIR}/PlugInUtilities/Interfaces
   ${Opticks_SOURCE_DIR}/PlugInUtilities/pthreads-wrapper DESTINATION
   include/PlugInUtilities FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE
   PATTERN "_svn" EXCLUDE)
install(DIRECTORY ${Opticks_SOURCE_DIR}/PlugIns/src/Aspam
   ${Opticks_SOURCE_DIR}/PlugIns/src/PlugInSampler
   ${Opticks_SOURCE_DIR}/PlugIns/src/PlugInSamplerHdf
   ${Opticks_SOURCE_DIR}/PlugIns/src/PlugInSamplerQt
   ${Opticks_SOURCE_DIR}/PlugIns/src/Tutorial DESTINATION examples/PlugIns/src
   FILES_MATCHING PATTERN "*" PATTERN ".svn" PATTERN ".git" EXCLUDE PATTERN "_svn" EXCLUDE)
install(DIRECTORY ${Opticks_SOURCE_DIR}/cmake/Modules DESTINATION cmake FILES_MATCHING
   PATTERN "*" PATTERN ".svn" PATTERN ".git" EXCLUDE PATTERN "_svn" EXCLUDE)
