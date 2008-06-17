
from os.path import join
import os.path
import shutil
import os

__all__ = ["get_dependencies"]

def isSubversionSoftLink(srcname):
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
         the_file = os.path.abspath(os.path.join(the_dir, the_linked_file))
   return the_file

def copy_dependencies(dp_list, dest_dir):
   if not(os.path.exists(dest_dir)):
      os.makedirs(dest_dir)
   for the_file,the_dest_dir in dp_list:
      the_file = os.path.abspath(the_file)
      file_name = os.path.split(the_file)[1]
      full_dest_dir = join(dest_dir,the_dest_dir)
      if not(os.path.exists(full_dest_dir)):
         os.makedirs(full_dest_dir)
      the_linked_file = isSubversionSoftLink(the_file)
      if the_linked_file != None:
         the_file = the_linked_file
      shutil.copy2(the_file, os.path.abspath(join(full_dest_dir,file_name)))

def get_dependencies(dependencies_path, platform, is_debug, arch):
   #this will return a list of dependencies as file paths
   #depending on the platform, mode and arch being passed in
   dp = dependencies_path

   dp_list = list()
   if platform == "Solaris":
      dp_list.append([join(dp,"ffmpeg/solaris-sparc/libavcodec/libavcodec.so.51"),"."])
      dp_list.append([join(dp,"ffmpeg/solaris-sparc/libavformat/libavformat.so.50"),"."])
      dp_list.append([join(dp,"ffmpeg/solaris-sparc/libavutil/libavutil.so.49"),"."])
      dp_list.append([join(dp,"xqilla/lib/solaris-sparc/libxqilla.so.1"),"."])
      dp_list.append([join(dp,"qwt/lib/solaris-sparc/libqwt.so.5"),"."])
      dp_list.append([join(dp,"Xerces/lib/solaris-sparc/libxerces-c.so.27"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQt3Support.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtCore.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtGui.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtNetwork.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtOpenGL.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtScript.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtSql.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtSvg.so.4"),"."])
      dp_list.append([join(dp,"Qt/lib/solaris-sparc/libQtXml.so.4"),"."])
      dp_list.append([join(dp,"ShapeLib/lib/solaris-sparc/libshp.so"),"."])
      dp_list.append([join(dp,"Hdf5/lib/solaris-sparc/libhdf5.so.0"),"."])
      dp_list.append([join(dp,"Hdf5/lib/solaris-sparc/libsz.so.2"),"."])
      dp_list.append([join(dp,"Qt/plugins/solaris-sparc/imageformats/libqgif.so"),"imageformats"])
      dp_list.append([join(dp,"Qt/plugins/solaris-sparc/imageformats/libqjpeg.so"),"imageformats"])
      dp_list.append([join(dp,"Qt/plugins/solaris-sparc/imageformats/libqmng.so"),"imageformats"])
      dp_list.append([join(dp,"Qt/plugins/solaris-sparc/imageformats/libqsvg.so"),"imageformats"])
      dp_list.append([join(dp,"Qt/plugins/solaris-sparc/imageformats/libqtiff.so"),"imageformats"])
      dp_list.append([join(dp,"ossim/lib/solaris-sparc/libossim.so.1"),"."])
      dp_list.append([join(dp,"OoModtran/lib/solaris-sparc/libOoModtran.so"),"."])
      dp_list.append([join(dp,"ehs/lib/solaris-sparc/libehs.so.0"),"."])
   elif platform == "Windows":
      temp_list = list()
      temp_list.append(dependency_suffix(r"Xerces\bin",["xerces-c_2_7"],"D.dll",".dll"))
      temp_list.append(dependency_suffix(r"xqilla\bin",["xqilla10"],"d.dll",".dll"))
      temp_list.append(dependency_suffix(r"glew\bin",["glew32"],"d.dll",".dll"))
      temp_list.append(dependency_suffix(r"Qt\bin",["Qt3Support","QtCore","QtGui","QtNetwork","QtOpenGL","QtScript","QtSql","QtSvg","QtXml"],"d4.dll","4.dll"))
      temp_list.append(dependency_suffix(r"Qt\plugins",[r"imageformats\qgif",r"imageformats\qjpeg",r"imageformats\qmng",r"imageformats\qsvg",r"imageformats\qtiff"],"d4.dll","4.dll",None,None,"imageformats"))
      temp_list.append(dependency_dir(r"qwt\bin",["qwt5.dll"],True))
      temp_list.append(dependency_dir(r"ShapeLib\bin",["shapelib.dll"]))
      temp_list.append(dependency_dir(r"pthreads\bin",["pthreadVC2.dll"]))
      temp_list.append(dependency_dir(r"Cg\bin",["cg.dll","cgc.exe","cgD3D9.dll","cgGL.dll"],False,["cgD3D8.dll","glut32.dll"]))
      temp_list.append(dependency_suffix(r"Hdf4\bin",[],"d.dll",".dll",["hd421m","hm421m"]))
      temp_list.append(dependency_suffix(r"Hdf4\bin",[],"","",["szlibdll.dll"]))
      temp_list.append(dependency_suffix(r"Hdf5\bin",["hdf5"],"dll.dll","dll.dll"))
      temp_list.append(dependency_suffix(r"zlib\bin",["zlib1"],"d.dll",".dll"))
      temp_list.append(dependency_suffix(r"zlib\bin",[],"","",["zlib1.dll"],["zlib1.dll"]))
      temp_list.append(dependency_dir(r"ffmpeg\Windows\build",["avcodec.dll","avformat.dll","avutil.dll"], True))
      temp_list.append(dependency_suffix(r"ossim\bin",["ossim"],"d.dll",".dll"))
      temp_list.append(dependency_suffix(r"ehs\bin",["ehs"],"d.dll",".dll"))

      for depend in temp_list:
         cur_list = depend.getListFor(arch, is_debug)
         for item,dest in cur_list: 
            dp_list.append([join(dp,item),dest])

   return dp_list

#the depedencies are dir\[Platform]\dll_name[modesuffix]
class dependency_suffix:
   def __init__(self, dir, dlls, debug_suffix, release_suffix, dllsForWin32 = None, dllsForWin64 = None, dest = "."):
      self.dir = dir
      self.dlls = dlls
      self.debug_suffix = debug_suffix
      self.release_suffix = release_suffix
      self.dllsForWin32 = dllsForWin32
      self.dllsForWin64 = dllsForWin64
      self.dest = dest
   def getListFor(self, platform, is_debug):
      if is_debug:
         suffix = self.debug_suffix
      else:
         suffix = self.release_suffix
      temp_list = list()
      dll_list = self.dlls[:]
      if (self.dllsForWin32 and platform == "Win32"):
         dll_list.extend(self.dllsForWin32)
      if (self.dllsForWin64 and platform == "x64"):
         dll_list.extend(self.dllsForWin64)
      for the_dll in dll_list:
         temp_list.append([join(self.dir, platform, the_dll + suffix),self.dest])
      return temp_list
      
#the dependencies are dir\[Platform]\dll's
class dependency_dir:
   def __init__(self, dir, dlls, dirForMode = False, dllsForWin32 = None, dllsForWin64 = None, dest = "."):
      self.dir = dir
      self.dlls = dlls
      self.dirForMode = dirForMode
      self.dllsForWin32 = dllsForWin32
      self.dllsForWin64 = dllsForWin64
      self.dest = dest
   def getListFor(self, platform, is_debug):
      temp_list = list()
      dll_list = self.dlls[:]
      if self.dllsForWin32 and platform == "Win32":
         dll_list.extend(self.dllsForWin32)
      if self.dllsForWin64 and platform == "x64":
         dll_list.extend(self.dllsForWin64)
      for the_dll in dll_list:
         dir = join(self.dir, platform)          
         if self.dirForMode:
            if is_debug:
               mode = "debug"
            else:
               mode = "release"
            dir = join(dir, mode)
         temp_list.append([join(dir, the_dll),self.dest])
      return temp_list
