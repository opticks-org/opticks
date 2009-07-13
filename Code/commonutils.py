
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
    def ds(the_dir, dlls, debug_suffix, release_suffix,
           dlls_for_win32 = None, dlls_for_win64 = None, dest = "."):
        temp_list.append(DependencySuffix(the_dir, dlls, debug_suffix,
            release_suffix, dlls_for_win32, dlls_for_win64, dest))
    def dd(the_dir, dlls, dir_for_mode = False,
           dlls_for_win32 = None, dlls_for_win64 = None, dest = "."):
        temp_list.append(DependencyDir(the_dir, dlls, dir_for_mode,
            dlls_for_win32, dlls_for_win64, dest))
    #this will return a list of dependencies as file paths
    #depending on the platform, mode and arch being passed in
    dp = dependencies_path

    dp_list = list()
    if platform == "Solaris" or platform == "Linux":
        if platform == "Solaris":
            plat_dir = "solaris-sparc"
        else:
            plat_dir = "linux-x86_64"
            la("gdal/lib/%s/libgdal.so.1" % (plat_dir))
            la("Cg/lib/%s/libCg.so" % (plat_dir))
            la("Cg/lib/%s/libCgGL.so" % (plat_dir))
            la("glew/lib/%s/libGLEW.so.1.3" % (plat_dir))
        la("ffmpeg/%s/libavcodec/libavcodec.so.51" % (plat_dir))
        la("ffmpeg/%s/libavformat/libavformat.so.50" % (plat_dir))
        la("ffmpeg/%s/libavutil/libavutil.so.49" % (plat_dir))
        la("xqilla/lib/%s/libxqilla.so.1" % (plat_dir))
        la("qwt/lib/%s/libqwt.so.5" % (plat_dir))
        la("Xerces/lib/%s/libxerces-c.so.27" % (plat_dir))
        la("Qt/lib/%s/libQt3Support.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtCore.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtGui.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtNetwork.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtOpenGL.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtScript.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtSql.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtSvg.so.4" % (plat_dir))
        la("Qt/lib/%s/libQtXml.so.4" % (plat_dir))
        la("ShapeLib/lib/%s/libshp.so" % (plat_dir))
        la("Hdf5/lib/%s/libhdf5.so.0" % (plat_dir))
        la("Hdf5/lib/%s/libsz.so.2" % (plat_dir))
        la("Qt/plugins/%s/imageformats/libqgif.so" % (plat_dir), "imageformats")
        la("Qt/plugins/%s/imageformats/libqjpeg.so" % (plat_dir), "imageformats")
        la("Qt/plugins/%s/imageformats/libqmng.so" % (plat_dir), "imageformats")
        la("Qt/plugins/%s/imageformats/libqsvg.so" % (plat_dir), "imageformats")
        la("Qt/plugins/%s/imageformats/libqtiff.so" % (plat_dir), "imageformats")
        la("ossim/lib/%s/libossim.so.1" % (plat_dir))
        la("gdal/lib/%s/libgdal.so.1" % (plat_dir))
        la("ehs/lib/%s/libehs.so.0" % (plat_dir))
    elif platform == "Windows":
        temp_list = list()
        ds(r"Xerces\bin", ["xerces-c_2_7"], "D.dll", ".dll")
        ds(r"xqilla\bin", ["xqilla10"], "d.dll", ".dll")
        ds(r"glew\bin", ["glew32"], "d.dll", ".dll")
        ds(r"Qt\bin",
            ["Qt3Support", "QtCore", "QtGui", "QtNetwork",
             "QtOpenGL", "QtScript", "QtSql", "QtSvg", "QtXml"],
            "d4.dll", "4.dll")
        ds(r"Qt\plugins",
            [r"imageformats\qgif", r"imageformats\qjpeg",
             r"imageformats\qmng", r"imageformats\qsvg",
             r"imageformats\qtiff"],
            "d4.dll", "4.dll", None, None, "imageformats")
        dd(r"qwt\bin", ["qwt5.dll"], True)
        dd(r"ShapeLib\bin", ["shapelib.dll"])
        dd(r"pthreads\bin", ["pthreadVC2.dll"])
        dd(r"Cg\bin", ["cg.dll", "cgc.exe", "cgD3D9.dll", "cgGL.dll"],
            False, ["cgD3D8.dll", "glut32.dll"])
        ds(r"Hdf4\bin",["hd423m","hm423m"],"d.dll",".dll")
        ds(r"Hdf4\bin", ["szlibdll.dll"],"","")
        ds(r"Hdf5\bin", ["hdf5"], "dll.dll", "dll.dll")
        ds(r"zlib\bin", ["zlib1"], "d.dll", ".dll")
        ds(r"zlib\bin", [], "", "", ["zlib1.dll"], ["zlib1.dll"])
        dd(r"ffmpeg\Windows\build",
            ["avcodec.dll", "avformat.dll", "avutil.dll"], True)
        ds(r"ossim\bin", ["ossim"], "d.dll", ".dll")
        ds(r"ehs\bin", ["ehs"], "d.dll", ".dll")
        ds(r"raptor\bin", ["raptor"], ".dll", ".dll")
        ds(r"expat\bin", ["libexpat"], ".dll", ".dll")
        ds(r"gdal\bin", ["gdal15"], ".dll", ".dll")

        for depend in temp_list:
            cur_list = depend.get_list_for(arch, is_debug)
            for item, dest in cur_list:
                la(item, dest)

    return dp_list

#the depedencies are dir\[Platform]\dll_name[modesuffix]
class DependencySuffix:
    def __init__(self, the_dir, dlls, debug_suffix, release_suffix,
                 dlls_for_win32, dlls_for_win64, dest):
        self.dir = the_dir
        self.dlls = dlls
        self.debug_suffix = debug_suffix
        self.release_suffix = release_suffix
        self.dlls_for_win32 = dlls_for_win32
        self.dlls_for_win64 = dlls_for_win64
        self.dest = dest
    def get_list_for(self, arch, is_debug):
        if is_debug:
            suffix = self.debug_suffix
        else:
            suffix = self.release_suffix
        temp_list = list()
        dll_list = self.dlls[:]
        if (self.dlls_for_win32 and arch == "32"):
            dll_list.extend(self.dlls_for_win32)
        if (self.dlls_for_win64 and arch == "64"):
            dll_list.extend(self.dlls_for_win64)
        if arch == "32":
            expand_arch = "Win32"
        elif arch == "64":
            expand_arch = "x64"
        for the_dll in dll_list:
            temp_list.append([join(self.dir, expand_arch, the_dll + suffix),
                self.dest])
        return temp_list

#the dependencies are dir\[Platform]\dll's
class DependencyDir:
    def __init__(self, the_dir, dlls, dir_for_mode,
                 dlls_for_win32, dlls_for_win64, dest):
        self.dir = the_dir
        self.dlls = dlls
        self.dir_for_mode = dir_for_mode
        self.dlls_for_win32 = dlls_for_win32
        self.dlls_for_win64 = dlls_for_win64
        self.dest = dest
    def get_list_for(self, arch, is_debug):
        temp_list = list()
        dll_list = self.dlls[:]
        if self.dlls_for_win32 and arch == "32":
            dll_list.extend(self.dlls_for_win32)
        if self.dlls_for_win64 and arch == "64":
            dll_list.extend(self.dlls_for_win64)
        if arch == "32":
            expand_arch = "Win32"
        elif arch == "64":
            expand_arch = "x64"
        for the_dll in dll_list:
            the_dir = join(self.dir, expand_arch)
            if self.dir_for_mode:
                if is_debug:
                    mode = "debug"
                else:
                    mode = "release"
                the_dir = join(the_dir, mode)
            temp_list.append([join(the_dir, the_dll), self.dest])
        return temp_list

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
