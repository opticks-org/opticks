
from os.path import join
import os.path
import shutil
import os
import re
import datetime

__all__ = ["get_dependencies", "get_app_version_only"]

class VersionException(Exception):
    """Report problems updating app version"""

def update_app_version(old_version, scheme, new_version, build_revision):
    version_number = old_version
    if new_version is not None:
        version_number = new_version

    if scheme == "nightly" or scheme == "unofficial":
        #strip off any suffix from the version #
        version_parts = version_number.split(".")
        if len(version_parts) >= 2:
            #Check for Nightly.BuildRev where BuildRev is just a number
            #If so, strip off the BuildRev portion so the rest off the
            #suffix stripping will work.
            if version_parts[-2].find("Nightly") != -1:
                version_parts = version_parts[0:-1] #Trim off the BuildRev part

        for count in range(0, len(version_parts) - 1):
            if not(version_parts[count].isdigit()):
                raise VersionException("The current app version # "\
                    "is improperly formatted.")
        last_part = version_parts[-1]
        match_obj = re.match("^\d+(\D*)", last_part)
        if match_obj is None:
            raise VersionException("The current app version # is "\
                "improperly formatted.")
        version_parts[-1] = last_part[:match_obj.start(1)]
        version_number = ".".join(version_parts)

        #append on the appropriate suffix to the version #
        if scheme == "unofficial":
            version_number = version_number + "Unofficial"
        elif scheme == "nightly":
            todays_date = datetime.date.today()
            today_str = todays_date.strftime("%Y%m%d")
            if len(today_str) != 8:
                raise VersionException("This platform does not properly "\
                    "pad month and days to 2 digits when using "\
                    "strftime.  Please update this script to address "\
                    "this problem")
            if not(str(build_revision).isdigit()):
                raise VersionException("The Build Revision when using "\
                    "--update-version=nightly must indicate a "\
                    "subversion working copy that has not been modified.")
            version_number = version_number + \
                "Nightly%s.%s" % (today_str, build_revision)
    elif new_version is None:
        print "You need to use --new-version to provide the version "\
            "# when using the production, rc, or milestone scheme"

    return version_number

def get_app_version_only(opticks_code_folder):
    app_version_path = join(opticks_code_folder, "application",
        "PlugInUtilities", "AppVersion.h")
    if not(os.path.exists(app_version_path)):
        return None
    app_version = open(app_version_path, "rt")
    version_info = app_version.read()
    app_version.close()

    version_number_match = re.search(r'APP_VERSION_NUMBER +?"(.*?)"',
        version_info)
    if version_number_match is not None:
        version_number = version_number_match.group(1)
        return version_number

    return None

def is_subversion_soft_link(srcname):
    the_file = None
    file_size = os.path.getsize(srcname)
    if file_size < 500:
        #open this file and determine if it was a soft link
        #when it was checked into Subversion
        the_file_contents = open(srcname)
        first_line = the_file_contents.readline()
        the_file_contents.close()
        if first_line.startswith("link"):
            the_linked_file = first_line.split(" ", 2)[1]
            the_dir = os.path.split(srcname)[0]
            the_file = os.path.abspath(join(the_dir, the_linked_file))
    return the_file

def copy_dependencies(dp_list, dest_dir):
    if not(os.path.exists(dest_dir)):
        os.makedirs(dest_dir)
    create_qt_conf(dest_dir, 1)
    for the_file, the_dest_dir in dp_list:
        the_file = os.path.abspath(the_file)
        file_name = os.path.split(the_file)[1]
        full_dest_dir = join(dest_dir, the_dest_dir)
        if not(os.path.exists(full_dest_dir)):
            os.makedirs(full_dest_dir)
        the_linked_file = is_subversion_soft_link(the_file)
        if the_linked_file is not None:
            the_file = the_linked_file
        shutil.copy2(the_file, os.path.abspath(join(full_dest_dir, file_name)))

def get_dependencies(dependencies_path, platform, is_debug, arch):
    def la(dp_file, dest_dir="."):
        dp_list.append([join(dp, dp_file), dest_dir])
    def bla(dp_file, dest_dir="."):
        dp_list.append([join(dp, "bin", dp_file), dest_dir])
    def lla(dp_file, dest_dir="."):
        dp_list.append([join(dp, "lib", dp_file), dest_dir])
    #this will return a list of dependencies as file paths
    #depending on the platform, mode and arch being passed in
    dp = join(dependencies_path, arch)

    dp_list = list()
    if platform == "Solaris" or platform == "Linux":
        if platform == "Solaris":
            plat_dir = "solaris-sparc"
            lla("libshp.so")
            lla("libgdal.so.1")
        else:
            plat_dir = "linux-x86_64"
            lla("libCg.so")
            lla("libCgGL.so")
            lla("libGLEW.so")
            lla("libshp.so.1")
            lla("libgdal.so")
            lla("libGLEW.so.1.5")
            lla("libopencv_highgui.so.2.2")
        lla("libavcodec.so.51")
        lla("libavformat.so.50")
        lla("libavutil.so.49")
        lla("libxqilla.so.5")
        lla("libqwt.so.5")
        lla("libxerces-c-3.1.so")
        lla("libQt3Support.so.4")
        lla("libQtCore.so.4")
        lla("libQtGui.so.4")
        lla("libQtNetwork.so.4")
        lla("libQtOpenGL.so.4")
        lla("libQtScript.so.4")
        lla("libQtSql.so.4")
        lla("libQtSvg.so.4")
        lla("libQtXml.so.4")
        lla("libhdf5.so.6")
        lla("libsz.so.2")
        la(join("plugins", "imageformats", "libqgif.so"), "imageformats")
        la(join("plugins", "imageformats", "libqjpeg.so"), "imageformats")
        la(join("plugins", "imageformats", "libqmng.so"), "imageformats")
        la(join("plugins", "imageformats", "libqsvg.so"), "imageformats")
        la(join("plugins", "imageformats", "libqtiff.so"), "imageformats")
        lla("libossim.so.1")
        lla("libcurl.so.4")
        lla("libehs.so.0")
        lla("libopencv_core.so.2.2")
        lla("libopencv_imgproc.so.2.2")
        lla("libopencv_ml.so.2.2")
        lla("libopencv_features2d.so.2.2")
        lla("libopencv_video.so.2.2")
        lla("libopencv_objdetect.so.2.2")
        lla("libopencv_calib3d.so.2.2")
        lla("libopencv_flann.so.2.2")
        bla("opencv_haartraining")
        bla("opencv_traincascade")
        lla("libyaml-cpp.so.0.2")
    elif platform == "Windows":
        bla("pthreadVC2.dll")
        bla("zlib1.dll")
        bla("raptor.dll")
        bla("libexpat.dll")
        bla("shapelib.dll")
        bla("hd425m.dll")
        bla("hm425m.dll")
        bla("szlibdll.dll")
        bla("gdal18.dll")
        bla("cg.dll")
        bla("cgc.exe")
        if arch == "32":
            bla("cgD3D8.dll")
            bla("openjpeg.dll")
        bla("cgD3D9.dll")
        bla("cgD3D10.dll")
        bla("cgGL.dll")
        bla("glut32.dll")
        bla("xerces-c_3_1.dll") #needed for GDAL
        bla("hdf5dll.dll") #needed for GDAL
        if is_debug:
            bla("ehsd.dll")
            bla("opencv_core220d.dll")
            bla("opencv_imgproc220d.dll")
            bla("opencv_ml220d.dll")
            bla("opencv_features2d220d.dll")
            bla("opencv_video220d.dll")
            bla("opencv_objdetect220d.dll")
            bla("opencv_calib3d220d.dll")
            bla("opencv_flann220d.dll")
            bla("tbb_debug.dll")
            bla("xerces-c_3_1D.dll")
            bla("xqilla22d.dll")
            bla("glew32d.dll")
            bla(join("debug", "qwt5.dll"))
            bla(join("debug", "avcodec.dll"))
            bla(join("debug", "avformat.dll"))
            bla(join("debug", "avutil.dll"))
            bla("ossimd.dll")
            bla("hdf5ddll.dll")
            la(join("plugins", "imageformats", "qgifd4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qjpegd4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qmngd4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qsvgd4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qtiffd4.dll"), "imageformats")
            la(join("lib", "Qt3Supportd4.dll"))
            la(join("lib", "QtCored4.dll"))
            la(join("lib", "QtGuid4.dll"))
            la(join("lib", "QtNetworkd4.dll"))
            la(join("lib", "QtOpenGLd4.dll"))
            la(join("lib", "QtScriptd4.dll"))
            la(join("lib", "QtSqld4.dll"))
            la(join("lib", "QtSvgd4.dll"))
            la(join("lib", "QtXmld4.dll"))
        else:
            bla("ehs.dll")
            bla("opencv_haartraining.exe")
            bla("opencv_traincascade.exe")
            bla("opencv_core220.dll")
            bla("opencv_imgproc220.dll")
            bla("opencv_ml220.dll")
            bla("opencv_features2d220.dll")
            bla("opencv_video220.dll")
            bla("opencv_objdetect220.dll")
            bla("opencv_calib3d220.dll")
            bla("opencv_flann220.dll")
            bla("tbb.dll")
            bla("xqilla22.dll")
            bla("glew32.dll")
            bla("qwt5.dll")
            bla("avcodec.dll")
            bla("avformat.dll")
            bla("avutil.dll")
            bla("ossim.dll")
            bla("hdf5dll.dll")
            la(join("plugins", "imageformats", "qgif4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qjpeg4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qmng4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qsvg4.dll"), "imageformats")
            la(join("plugins", "imageformats", "qtiff4.dll"), "imageformats")
            la(join("lib", "Qt3Support4.dll"))
            la(join("lib", "QtCore4.dll"))
            la(join("lib", "QtGui4.dll"))
            la(join("lib", "QtNetwork4.dll"))
            la(join("lib", "QtOpenGL4.dll"))
            la(join("lib", "QtScript4.dll"))
            la(join("lib", "QtSql4.dll"))
            la(join("lib", "QtSvg4.dll"))
            la(join("lib", "QtXml4.dll"))
    return dp_list

def create_qt_conf(path, verbosity):
    qt_conf_file_path = join(path, "qt.conf")
    if not(os.path.exists(qt_conf_file_path)):
        if not(os.path.exists(path)):
            os.makedirs(path)
        if verbosity > 1:
            print "Creating qt.conf file..."
        qt_conf_file = open(qt_conf_file_path, "w")
        qt_conf_file.write("[Paths]\n"\
            "Plugins = .")
        qt_conf_file.close()
        if verbosity > 1:
            print "Done creating qt.conf file"
