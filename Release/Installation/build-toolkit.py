#!/usr/local/bin/python
import os
from os.path import join
import os.path
import sys
from optparse import OptionParser
import traceback
import shutil
import subprocess
import zipfile

def cp_file2(source_base_dir, destination_base_dir, common_dir, file_to_copy):
   full_src_file = join(source_base_dir, common_dir, file_to_copy)
   full_dst_dir = join(destination_base_dir, common_dir)
   return cp_file(full_src_file, full_dst_dir)

def cp_file(source_file, destination_dir):
   """Copy specified file to destination dir, creating all necessary folders
      in destination if needed. 

    src_file -- path of source file 
    dst_dir -- path of destination directory

   """
   if not(os.path.exists(destination_dir)):
      os.makedirs(destination_dir)
   shutil.copy2(source_file, join(destination_dir, os.path.basename(source_file)))

def cp_file3(source_file, destination_file):
   destination_dir = os.path.dirname(destination_file)
   if not(os.path.exists(destination_dir)):
      os.makedirs(destination_dir)
   shutil.copy2(source_file, destination_file)

def cp_dir2(source_base_dir, destination_base_dir, dir_to_copy, suffixes_to_match = [], exclude_matches = False, recursive_copy = False, dirs_to_exclude = [".svn", "_svn"]):
   full_src_dir = join(source_base_dir, dir_to_copy)
   full_dst_dir = join(destination_base_dir, dir_to_copy)
   return cp_dir(full_src_dir, full_dst_dir, suffixes_to_match, exclude_matches, recursive_copy, dirs_to_exclude)

def cp_dir(source_dir, destination_dir, suffixes_to_match = [], exclude_matches = False, recursive_copy = False, dirs_to_exclude = [".svn", "_svn"]):
   """Copy all files with or without specified suffix in source directory to
      destination directory

    @param source_dir: path of source directory
    @type source_dir: L{str}
    @param destination_dir: path of destination directory
    @type destination_dir: L{str} 
    @param suffixes_to_match: a list of the specified suffixes to match before files are copied.  This may
              be an empty list and all files will match and therefore be copied.
    @type suffixes_to_match: L{list} of L{str}
    @param exclude_matches: if False, only files that end with suffix will be copied.  If
              True, only files that don't end with suffix will be copied.
    @type exclude_matches: L{bool} 
    @param recursive_copy: If True, the copy will be performed recursively.  Note that the
    suffix to match only applies to files and not directories.
    @type recursive_copy: L{bool}
    @param dirs_to_exclude: List of directories that should be excluded when recursion is applied.
    @type dirs_to_exclude: L{list} of L{str}

    This function will create destination directory if it does not
    exist.
   """
   dir_contents = os.listdir(source_dir)
   symlinks = False
   if not(os.path.exists(destination_dir)):
      os.makedirs(destination_dir)
   for entry in dir_contents:
      src_path = os.path.join(source_dir, entry)
      dst_path = os.path.join(destination_dir, entry)
      if symlinks and os.path.islink(src_path):
         linkto = os.readlink(src_path)
         os.symlink(linkto, dst_path)
      elif os.path.isdir(src_path):
         if recursive_copy and dirs_to_exclude.count(entry) == 0:
            cp_dir(src_path, dst_path, suffixes_to_match, exclude_matches, recursive_copy, dirs_to_exclude)
      else:
         matches = False 
         if suffixes_to_match == None or len(suffixes_to_match) == 0:
            matches = True
         else:
            for suffix in suffixes_to_match:
               if entry.endswith(suffix) != exclude_matches:
                  matches = True

         if matches:
            source_file = src_path
            if symlinks:
               the_file = commonutils.isSubversionSoftLink(source_file)
               if the_file != None:
                  source_file = the_file 
            
            shutil.copy2(source_file, dst_path)
      
def copyWindowsBuild(opticks_code_dir,sdk_dest_dir,win_debug_dest_dir,libs,plugins,is_32_bit,is_debug):
   if is_32_bit:
      arch = "Win32"
   else:
      arch = "x64"
   if is_debug:
      mode = "debug"
   else:
      mode = "release"
   binaries_dir = join("Build","Binaries-%s-%s" % (arch,mode))

   executables = ["Opticks","OpticksBatch"]
   for the_exec in executables:
      cp_file2(opticks_code_dir, sdk_dest_dir, join(binaries_dir,"Bin"), the_exec+".exe")

   for the_lib in libs:
      cp_file2(opticks_code_dir, sdk_dest_dir, join(binaries_dir,"Lib"), the_lib + ".lib")

   for the_plugin in plugins:
      cp_file2(opticks_code_dir, sdk_dest_dir, join(binaries_dir,"PlugIns"), the_plugin + ".dll")

   cp_dir2(opticks_code_dir, sdk_dest_dir, join(binaries_dir,"PlugIns","ArcProxy"), suffixes_to_match = [".dll", ".exe"])

   if is_debug:
      #Copy the pdbs for the libs
      all_pdbs = executables + libs + plugins
      for the_file in all_pdbs:
         pdbs_dir = join(binaries_dir,"pdbs") 
         cp_file2(opticks_code_dir, win_debug_dest_dir, pdbs_dir, the_file + ".pdb") 

def copyDirIntoZip(zip_file, parent_src_dir, the_dir, prefix_dir, keep_the_dir=True):
   src_dir = join(parent_src_dir, the_dir)
   for root, dirs, files in os.walk(src_dir):
      try:
         dirs.remove(".svn")
      except:
         pass
      try:
         dirs.remove("_svn")
      except:
         pass
      if keep_the_dir:
         the_zip_dir = root[len(parent_src_dir)+1:]
      else:
         the_zip_dir = root[len(parent_src_dir) + len(the_dir) + 1:]
      for the_file in files:
         source_file = join(root,the_file)
         linked_file = commonutils.isSubversionSoftLink(source_file)
         if linked_file != None:
            source_file = linked_file
         zip_file.write(source_file, join(prefix_dir,the_zip_dir,the_file))
         
def create_toolkit_zip(opticks_code_dir, opticks_dependencies_dir, package_dir):
   if opticks_dependencies_dir == None:
      if os.environ.has_key("OPTICKSDEPENDENCIES"):
         opticks_dependencies_dir = os.environ["OPTICKSDEPENDENCIES"]
      else:
         print "ERROR: The path to the Opticks dependencies was not provided, see -d"
         return 1001

   if not(os.path.exists(opticks_dependencies_dir)):
      print "ERROR: The path to the Opticks dependencies does not exist %s, see -d"

   out_dir = os.path.abspath(join("Toolkit", "SDK-Temp"))
   win_debug_dir = os.path.abspath(join("Toolkit", "WinDebug-Temp"))
   if os.path.exists(out_dir):
      shutil.rmtree(out_dir, False)

   if os.path.exists(win_debug_dir):
      shutil.rmtree(win_debug_dir, False)
      
   ##### Create all the output directories
   os.makedirs(out_dir)
   os.makedirs(win_debug_dir)

   cp_file3("README-sdk.txt", os.path.join(out_dir, "README.txt"))
   s_app = os.path.abspath(os.path.join(opticks_code_dir, "application"))
   s_release = os.path.abspath(os.path.join(opticks_code_dir, "Release"))
   d_app = join(out_dir,"Application")

   interface_suffixes = [".h"]
   cp_dir2(s_app, d_app, "Interfaces", suffixes_to_match=interface_suffixes)
   cp_dir2(s_app, d_app, join("PlugInUtilities","Interfaces"), suffixes_to_match=interface_suffixes)
   cp_dir2(s_app, d_app, join("PlugInUtilities","pthreads-wrapper"), suffixes_to_match=interface_suffixes)
   if is_windows():
      compile_settings_suffix = [".vsprops"]
   else:
      compile_settings_suffix = [".py"]
   cp_dir2(s_app, d_app, "CompileSettings", suffixes_to_match=compile_settings_suffix)
   cp_dir2(s_app, d_app, "PlugInLib", suffixes_to_match=interface_suffixes)
   cp_dir2(s_app, d_app, "HdfPlugInLib", suffixes_to_match=interface_suffixes)
   cp_dir2(s_app, d_app, "NitfPlugInLib", suffixes_to_match=interface_suffixes)
   
   #Copy the PlugInSamplerQt code to the right spot
   source_suffixes = interface_suffixes + [".cpp", ".ui"]
   if is_windows():
      source_suffixes.append(".vcproj")
   else:
      source_suffixes.append("SConscript")

   cp_dir2(s_app, d_app, join("PlugIns","src","PlugInSamplerQt"), suffixes_to_match=source_suffixes)
   cp_dir2(s_app, d_app, join("PlugIns","src","PlugInSampler"), suffixes_to_match=source_suffixes)
   cp_dir2(s_app, d_app, join("PlugIns","src","PlugInSamplerHdf"), suffixes_to_match=source_suffixes)
   cp_dir2(s_app, d_app, join("PlugIns","src","Tutorial"), suffixes_to_match=source_suffixes)

   #Copy the ModuleManager.cpp to the right spot in the Toolkit
   cp_file(join(s_release, "Toolkit", "src", "ModuleManager.cpp"), join(out_dir, "Src"))

   win_debug_code_dir = os.path.join(win_debug_dir, "Code")
   if is_windows():
      svn_export_code_args = list()
      svn_export_code_args.append("svn")
      svn_export_code_args.append("export")
      svn_export_code_args.append("-r")
      svn_export_code_args.append("BASE")
      svn_export_code_args.append(os.path.abspath(opticks_code_dir))
      svn_export_code_args.append(win_debug_code_dir)
      svn_export_code = subprocess.Popen(svn_export_code_args)
      retcode = svn_export_code.wait()
      if retcode != 0:
         print "ERROR: Unable to export code."

      cp_file3("README-pdb-source.txt", os.path.join(win_debug_dir, "README.txt"))


   #Copy dependencies
   svn_export_dependencies_args = list()
   svn_export_dependencies_args.append("svn")
   svn_export_dependencies_args.append("export")
   svn_export_dependencies_args.append("-r")
   svn_export_dependencies_args.append("BASE")
   svn_export_dependencies_args.append(os.path.abspath(opticks_dependencies_dir))
   svn_export_dependencies_args.append(join(out_dir, "Dependencies"))
   svn_export_dependencies = subprocess.Popen(svn_export_dependencies_args)
   retcode = svn_export_dependencies.wait()
   if retcode != 0:
      print "ERROR: Unable to export dependencies"

   ##### Run Doxygen to generate the html documentation
   retcode = subprocess.Popen([join(opticks_code_dir,"build.py"), "--build-doxygen"], shell=True).wait()
   if retcode != 0:
      print "ERROR: Error occurred while building on-line help"
   if is_windows():
      #TODO - Get doxygen running on Solaris, currently it doesn't run
      cp_dir2(join(s_release, "Toolkit", "doc"), join(out_dir, "doc"), "html")
   else:
      print "WARNING: You must currently run this script on Windows to generate the doxygen and then copy the output over to this system."
 
   plugins_file = None
   libs = ["PlugInLib", "PlugInUtilities", "HdfPlugInLib", "NitfPlugInLib"]
   plugins = [ "Aspam", "AutoImporter", "BandMath", "CoreIo", "DataFusion", "Dted",
      "ENVI", "Generic", "GeographicFeatures", "Georeference", "Hdf", "Ice", "Kml",
      "MovieExporter", "Nitf", "NitfCommonTre", "Pca", "Pictures", "Results", 
      "Scripts", "SecondMoment", "ShapeFileExporter", "Sio", "WizardExecutor",
      "WizardItems" ]
   sample_plugins = ["PlugInSampler", "PlugInSamplerQt", "PlugInSamplerHdf", "ProductionPlugInTester", "Tutorial" ]
   if is_windows():
      cp_file2(s_app, d_app, "", "SamplePlugIns.sln")
      cp_file2(s_app, d_app, "PlugInManager", "PlugInModule.def")

      plugins = plugins + sample_plugins
      #Win32 Build
      copyWindowsBuild(opticks_code_dir,out_dir,win_debug_code_dir, libs,plugins,True,False)
      copyWindowsBuild(opticks_code_dir,out_dir,win_debug_code_dir,libs,plugins,True,True)

      dp_list = commonutils.get_dependencies(opticks_dependencies_dir,"Windows",True,"Win32")
      commonutils.copy_dependencies(dp_list, join(out_dir,"Build","Binaries-win32-debug","Bin"))
      dp_list = commonutils.get_dependencies(opticks_dependencies_dir,"Windows",False,"Win32")
      commonutils.copy_dependencies(dp_list, join(out_dir,"Build","Binaries-win32-release","Bin"))

      #Win64 Build
      copyWindowsBuild(opticks_code_dir,out_dir,win_debug_code_dir,libs,plugins,False,False)
      copyWindowsBuild(opticks_code_dir,out_dir,win_debug_code_dir,libs,plugins,False,True)
      dp_list = commonutils.get_dependencies(opticks_dependencies_dir,"Windows",True,"x64")
      commonutils.copy_dependencies(dp_list, join(out_dir,"Build","Binaries-x64-debug","Bin"))
      dp_list = commonutils.get_dependencies(opticks_dependencies_dir,"Windows",False,"x64")
      commonutils.copy_dependencies(dp_list, join(out_dir,"Build","Binaries-x64-release","Bin"))
   else:
      cp_file2(s_app, d_app, join("PlugIns", "src"), "SConstruct")

      binaries_dir = join("Build", "Binaries-solaris-sparc-release")
      lib_dir = join(binaries_dir,"Lib")
      for the_lib in libs:
         cp_file2(opticks_code_dir, out_dir, lib_dir, "lib%s.a" % (the_lib))
      cp_file2(opticks_code_dir, out_dir, lib_dir, "ModuleShell.os")
      
      for the_plugin in sample_plugins:
         cp_file2(opticks_code_dir, out_dir, join(binaries_dir,"PlugIns"), "%s.so" % (the_plugin))

   if package_dir != None and os.path.exists(package_dir):
      if is_windows():
         zip = zipfile.ZipFile(join(package_dir,"opticks-sdk-%s-windows.zip" % (commonutils.get_app_version_only(opticks_code_dir))), "w", zipfile.ZIP_DEFLATED)
         copyDirIntoZip(zip, os.path.abspath("Toolkit"), "SDK-Temp", ".", False)
         zip.close()

         zip = zipfile.ZipFile(join(package_dir,"opticks-pdb-sourcecode-%s-windows.zip" % (commonutils.get_app_version_only(opticks_code_dir))), "w", zipfile.ZIP_DEFLATED)
         copyDirIntoZip(zip, os.path.abspath("Toolkit"), "WinDebug-Temp", ".", False)
         zip.close()
      else:
         tar_args = list()
         tar_args.append("tar")
         tar_args.append("-cvf")
         tar_args.append("-")
         tar_args.append(".")
         tar = subprocess.Popen(tar_args, stdout=subprocess.PIPE, cwd=out_dir)

         output_tar_bz2 = os.path.abspath(os.path.join(package_dir, "opticks-sdk-%s-sol10-sparc.tar.bz2" % (commonutils.get_app_version_only(opticks_code_dir)))) 
         output_handle = open(output_tar_bz2, "wb")
         bzip2_args = list()
         bzip2_args.append("bzip2")
         bzip2_args.append("-c")
         bzip2 = subprocess.Popen(bzip2_args, stdin=tar.stdout, stdout=output_handle)

         tar_ret = tar.wait()
         bzip_ret = bzip2.wait()
         output_handle.close()
         if tar_ret != 0:
            return tar_ret
         if bzip_ret != 0:
            return bzip_ret

def is_windows():
   """Determine if this script is executing on the Windows operating system.
   @return: Return True if script is executed on Windows, False otherwise.
   @rtype: L{bool}

   """
   return sys.platform.startswith("win32")

def parse_args():
   ##### Parse the arguments
   if is_windows():
      desc = "Generate the Developers Toolkit .zip file for the 32-bit and 64-bit version of Opticks for Windows."
   else:
      desc = "Generate the Developers Toolkit file for the Solaris version of Opticks."

   parser = OptionParser(usage="%prog [options]", version="%prog 1.0", description=desc)
   parser.add_option("-c", "--code-dir", action="store", dest="opticks_code_dir",
         default=None, help="The path to the checkout of the Code folder from the Opticks trunk.")
   parser.add_option("-d", action="store", dest="dependencies", default=None, help="The path to Opticks dependencies")   
   parser.add_option("--package-dir", action="store", dest="package_dir", default=None, help="The directory where the toolkit output should be stored.  This directory must already exist.")

   #Parse the optional arguments, plus any additional arguments present
   #after optional arguments
   (options, args) = parser.parse_args()

   return options

#Main execution path when script is run
if __name__ == "__main__":
   args = parse_args()
   if args.opticks_code_dir == None:
      if os.environ.has_key("OPTICKS_CODE_DIR"):
         args.opticks_code_dir = os.environ["OPTICKS_CODE_DIR"]       
      else:
         print "ERROR: The path to the Opticks source code was not provided, see -c or --code-dir"
         sys.exit(1000)

   if not(os.path.exists(args.opticks_code_dir)):
      print "ERROR: The path to the Opticks source code does not exist %s, see -c or --code-dir"
      sys.exit(1001)

   sys.path.append(args.opticks_code_dir)
   import commonutils
   
   retcode = create_toolkit_zip(args.opticks_code_dir, args.dependencies, args.package_dir) 
   sys.exit(retcode)
