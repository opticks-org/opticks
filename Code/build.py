#!/usr/local/bin/python
import sys
import subprocess
import os
import os.path
from os.path import join
import optparse
import traceback
import shutil
import xml.dom.minidom
import datetime
import time
import zipfile
import calendar
import codecs
import re
import commonutils

#chdir to the directory where the script resides
os.chdir(os.path.abspath(os.path.dirname(sys.argv[0])))

class ScriptException:
    def __init__(self, msg, retcode):
        self.msg = msg
        self.retcode = retcode

class Builder:
    def __init__(self, dependencies, arcsdk, build_in_debug):
        self.depend_path = dependencies
        self.arcsdk_path = arcsdk
        self.build_debug_mode = build_in_debug
        if self.build_debug_mode:
            self.mode = "debug"
        else:
            self.mode = "release"

    def __get_app_version_only(self):
        return commonutils.get_app_version_only(".")

    def build_doxygen(self, artifactsDir):
        current_app_version = self.get_current_app_version()
        docPath = os.path.abspath(os.path.join("Build", "DoxygenOutput"))
        if os.path.exists(docPath):
           shutil.rmtree(docPath, True) #delete any already generated documentation
        os.makedirs(docPath)
        doxygen_cmd = self.getDoxygenPath() 
        config_dir = os.path.abspath(os.path.join("application", "ApiDocs")) 
        args = [os.path.join(config_dir, "application.dox")]
        env = os.environ
        env["SOURCE"] = os.path.abspath("application")
        env["VERSION"] = current_app_version 
        env["OUTPUT_DIR"] = docPath
        env["CONFIG_DIR"] = config_dir 
        graphviz_dir = os.path.abspath(os.path.join(self.depend_path, "graphviz", "app")) 
        env["DOT_DIR"] = os.path.join(graphviz_dir, "bin")
        if sys.platform.startswith("sunos"):
            env["GVBINDIR"] = os.path.join(graphviz_dir, "lib", "graphviz")
            new_value = os.path.join(graphviz_dir, "lib") 
            if env.has_key("LD_LIBRARY_PATH_32"):
               new_value = new_value + ":" + env["LD_LIBRARY_PATH_32"]
            env["LD_LIBRARY_PATH_32"] = new_value
        retCode = self.run_app(doxygen_cmd, ".", None, None, ".", env, args)
        if retCode != 0:
            raise ScriptException("Unable to run doxygen generation script", retCode)
        print "Done generating doxygen"
        if artifactsDir != None:
           print "Zipping doxygen"
           htmlPath = os.path.join(docPath, "html")
           zip_name = "doxygen.zip"
           zip_path = os.path.abspath(join(artifactsDir, zip_name))
           the_zip = zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED)
           for cur_dir, dirs, files in os.walk(htmlPath):
               arc_dir = cur_dir[len(htmlPath):]
               for the_file in files:
                   the_zip.write(join(cur_dir,the_file),join(arc_dir,the_file))
           the_zip.close()

    def __update_app_version_h(self, fields_to_replace):
        app_version = open(os.path.join("application", "PlugInUtilities", "AppVersion.h"), "rt")
        version_info = app_version.readlines()
        app_version.close()
        app_version = open(os.path.join("application", "PlugInUtilities", "AppVersion.h"), "wt")
        for vline in version_info:
            fields = vline.strip().split()
            if len(fields) >= 3 and fields[0] == '#define' and fields[1] in fields_to_replace:
                app_version.write('#define %s %s\n' % (fields[1], fields_to_replace[fields[1]]))
            else:
                app_version.write(vline)
        app_version.close()

        #update modification time to force compilation to occur
        #even in incremental mode
        os.utime(os.path.join("application", "Utilities", "ConfigurationSettingsImp.cpp"), None)
       
       
    def get_current_app_version(self):
        # Try to update the build revision, since it might be stale
        command_prefix = "./"
        if sys.platform.startswith("win"):
            command_prefix = ""
        update_build_rev = subprocess.Popen(["python", "%supdate-build-revision.py" % (command_prefix)], shell=False)
        update_build_rev.wait()

        # Read the current build revision directly from the file.
        build_revision_path = os.path.join("application", "Utilities", "BuildRevision.h")
        if not(os.path.exists(build_revision_path)):
            return "Unknown"
        build_revision_file = open(build_revision_path, "rt")
        revision_info = build_revision_file.read()
        build_revision_file.close()

        # Read the app version directly from the file
        version_number = self.__get_app_version_only()
        if version_number != None:
            build_revision_match = re.search(r'BUILD_REVISION +?"(.*?)"', revision_info)
            if build_revision_match != None:
                build_revision = build_revision_match.group(1) 

            # Concat the version number and build revision
            return version_number + " Build " + build_revision

        return "Unknown"

    def update_app_version_number(self, scheme, new_version, release_date):
        if scheme == None or scheme == "none":
            return

        # Read the app version directly from the file
        version_number = self.__get_app_version_only()
        if version_number == None:
            raise ScriptException("Could not determine the current app version while attempting to update the app version", 1500)
        print "Original version # of Opticks was", version_number
        version_number = commonutils.update_app_version(version_number, scheme, new_version)
        print "Setting version # of Opticks to", version_number

        # Update AppVersion.h
        fields_to_replace = dict()
        fields_to_replace["APP_VERSION_NUMBER"] = '"' + version_number + '"'
        if scheme == "production":
            fields_to_replace["APP_IS_PRODUCTION_RELEASE"] = "true"
            print "Making Opticks a production release"
        else:
            fields_to_replace["APP_IS_PRODUCTION_RELEASE"] = "false"
            print "Making Opticks a not for production release"

        date_obj = None
        if scheme == "nightly" or release_date == "today":
            date_obj = datetime.date.today()
        elif release_date != None:
            print release_date
            date_obj = None
            try:
                date_tuple = time.strptime(release_date, "%Y-%m-%d")
                date_obj = datetime.date(*date_tuple[0:3])
            except:
                print "ERROR: The release date is not in the proper format, use YYYY-MM-DD."

        if date_obj != None:
            fields_to_replace["APP_RELEASE_DATE_YEAR"] = str(date_obj.year)
            fields_to_replace["APP_RELEASE_DATE_MONTH"] = str(date_obj.month)
            fields_to_replace["APP_RELEASE_DATE_DAY"] = str(date_obj.day)
            print "Updating the release date to %s" % (date_obj.isoformat())
        else:
            print "The release date has not been updated"

        self.__update_app_version_h(fields_to_replace)

    def populate_environ_for_dependencies(self, env):
        env["OPTICKSDEPENDENCIES"] = self.depend_path
        if self.arcsdk_path:
            env["ARCSDK"] = self.arcsdk_path

    def build_executable(self, clean_build_first, build_opticks, concurrency):
        #No return code, throw exception or ScriptException
        if build_opticks == "none":
            return

        buildenv = os.environ
        self.populate_environ_for_dependencies(buildenv)

        if build_opticks != "core":
            if not "ARCSDK" in buildenv:
                raise ScriptException("ARCSDK is not provided", 1100)
            if not os.path.exists(buildenv.get("ARCSDK")):
                raise ScriptException("ArcSDK path does not exist", 1101)
            
        print_env(buildenv)
        sys.stdout.flush()

        if clean_build_first:                
            echo("Cleaning Opticks first")
            self.compile_code(buildenv, True, build_opticks, concurrency)
            
        echo("Building Opticks")
        self.compile_code(buildenv, False, build_opticks, concurrency)

    def gather_artifacts(self, artifactsDir):
        binaries_dir = os.path.abspath(self.getBinariesDir("Build"))
        zip_name = self.getZipName()
        zip_path = os.path.abspath(join(artifactsDir, zip_name))
        the_zip = zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED)
        for cur_dir, dirs, files in os.walk(binaries_dir):
            root = cur_dir[len(binaries_dir):]
            if os.path.isabs(root):
               root = apply(os.path.join, root.split(os.path.sep)[1:])
            for the_file in files:
                the_zip.write(join(cur_dir,the_file),join(root,the_file))
            
        the_zip.close()

    def run_app(self, executable, path, default_dir, config_dir, opticks_home=".", env=None, args=None,
                stdout_file = None):
        arguments = []          
        arguments.append(executable)

        new_env = os.environ
        new_env["OPTICKS_HOME"] = os.path.abspath(opticks_home)

        if env != None:
            #Copy environment variables over
            for key in env.iterkeys():
                new_env[key] = env[key]
        
        if args == None:
            if sys.platform.startswith("sunos"):
                prefix = "-"
            else:
                prefix = "/"
            if default_dir:
                arguments.append("%sdefaultDir:%s" % (prefix,default_dir))
            if config_dir:
                arguments.append("%sconfigDir:%s" % (prefix, config_dir))
            arguments.append("%scatchErrors:true" % (prefix))
        else:
            arguments.extend(args)

        oldCwdPath = os.getcwd()
        os.chdir(path)
        if sys.platform.startswith("sunos"):
            if stdout_file != None:
                newstdout = open(stdout_file, "w")
            else:
                newstdout = os.dup(sys.stdout.fileno())
            proc = subprocess.Popen(arguments, stdin=subprocess.PIPE, stdout=newstdout,
                                    stderr=subprocess.STDOUT, env=new_env)
        else:            
            proc = subprocess.Popen(arguments, stdin=subprocess.PIPE, stdout=sys.stdout)
        
        proc.stdin.close()
        retCode = proc.wait()
        os.chdir(oldCwdPath)

        return retCode

    def update_cfg_file(self, cfgfile, plugin_path):
        def pluginpath_repl(matchobj):
           return plugin_path

        # Update the plug-in path
        # using regular expressions over the .cfg file
        # contents
        if cfgfile == None:
            return
        
        cfgfileHandle = codecs.open(cfgfile, "r", "utf-8")
        contents = cfgfileHandle.read()
        cfgfileHandle.close()
        contents = re.sub("\$\$\$PLUGINPATH\$\$\$", pluginpath_repl, contents)
        cfgfileHandle = codecs.open(cfgfile, "w+", "utf-8")
        cfgfileHandle.write(contents)
        cfgfileHandle.close()

class WindowsBuilder(Builder):
    def __init__(self, dependencies, arcsdk, build_in_debug, visualstudio):
        Builder.__init__(self, dependencies, arcsdk, build_in_debug)
        self.vs_path = visualstudio
    
    def compile_code(self, env, clean, build_opticks, concurrency):
        solution_file = os.path.abspath("Application\\Opticks.sln")
        self.build_in_visual_studio(solution_file, self.build_debug_mode, self.is_64_bit, concurrency, self.vs_path, env, clean)
        if build_opticks != "core":
            solution_file = os.path.abspath("Application\\ArcIntegration.sln")
            self.build_in_visual_studio(solution_file, self.build_debug_mode, self.is_64_bit, concurrency, self.vs_path, env, clean)
            
    def getBinariesDir(self, build_dir):
        return join(build_dir,"Binaries-%s-%s" % (self.platform, self.mode))

    def getZipName(self):
        return "Binaries-%s-%s.zip" % (self.platform, self.mode)

    def getDoxygenPath(self):
        return os.path.join(self.depend_path, "doxygen", "bin", "doxygen.exe")
    
    def set_opticks_defaults(self, build_dir):
        config_defaults_dir = os.path.abspath(join(self.getBinariesDir(build_dir), "DefaultSettings"))
        if not os.path.exists(config_defaults_dir):
           os.makedirs(config_defaults_dir)
        plugin_path_defaults = join(config_defaults_dir, "100-PlugInPath-defaults.cfg")
        if not os.path.exists(plugin_path_defaults):
           shutil.copy2("PlugInPath-defaults.cfg", plugin_path_defaults) 

        plugin_path = os.path.abspath(join(self.getBinariesDir(build_dir),"PlugIns"))
        self.update_cfg_file(plugin_path_defaults, plugin_path) 

        return config_defaults_dir
   
    def prep_to_run(self, lib_path, default_dir, opticks_home, build_dir):
        dp = self.depend_path

        if build_dir == None:
            build_dir = "Build"
        dll_path = "DllsFor%s-%s" % (self.mode, self.platform)
        dll_path = os.path.abspath(join(lib_path, dll_path))
            
        if not os.path.exists(dll_path):
            os.makedirs(dll_path)

        dp_list = commonutils.get_dependencies(self.depend_path, "Windows", self.build_debug_mode, self.platform) 
        commonutils.copy_dependencies(dp_list, dll_path)

        qt_imageformat_plugins = ["qgif", "qjpeg", "qmng", "qsvg", "qtiff"]
        plugin_dest_dir = os.path.abspath(join(self.getBinariesDir(build_dir),"Bin","imageformats"))
        if not os.path.exists(plugin_dest_dir):
            os.makedirs(plugin_dest_dir)
        for qt_plugin in qt_imageformat_plugins:
            plugin_file = qt_plugin
            if self.build_debug_mode:
                plugin_file = plugin_file + "d4.dll"
            else:
                plugin_file = plugin_file + "4.dll"
            plugin_src = os.path.abspath(join(self.depend_path,"Qt","plugins",self.platform,"imageformats",plugin_file))
            shutil.copy2(plugin_src, os.path.abspath(join(plugin_dest_dir,plugin_file)))

        # Qt DLLs for ArcProxy
        proxy_dir = join(self.getBinariesDir(build_dir), "PlugIns", "ArcProxy")
        if not os.path.exists(proxy_dir):
            os.makedirs(proxy_dir)

        proxy_dll_names = []
        if self.build_debug_mode:
            proxy_dll_names = ["QtCored4.dll", "QtNetworkd4.dll"]
        else:
            proxy_dll_names = ["QtCore4.dll", "QtNetwork4.dll"]
        for proxy_dll_name in proxy_dll_names:
            shutil.copy2(join(self.depend_path, "Qt", "bin", "win32", proxy_dll_name), join(proxy_dir, proxy_dll_name))
        
        if self.build_debug_mode:
            #Runtime DLL's
            vs7_runtime_dlls = "C:\luntbuild-area\VisualStudio7RuntimeDlls\Debug"
            if os.path.exists(vs7_runtime_dlls):
                copy_files_in_dir(vs7_runtime_dlls,dll_path)
        else:
            #NOTE: This is a hack until Opticks 4 needs only Visual Studio 8
            #Runtime DLL's
            vs7_runtime_dlls = "C:\luntbuild-area\VisualStudio7RuntimeDlls\Release"
            if os.path.exists(vs7_runtime_dlls):
                copy_files_in_dir(vs7_runtime_dlls,dll_path)
        
        env = dict()
        env["PATH"] = dll_path

        if opticks_home == None:
            opticks_home = "./Release"
            
        temp_dir = join(opticks_home, "Temp")
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)

        if build_dir == None:
           build_dir = "Build"
        app_setting_dir = os.path.abspath(join(build_dir, "ApplicationUserSettings"))
        if not os.path.exists(app_setting_dir):
            os.makedirs(app_setting_dir)
        os.makedirs(temp_dir)

        if not default_dir:
            default_dir = self.set_opticks_defaults(build_dir)
            
        return (dll_path, default_dir)
                                 
    def build_in_visual_studio(self, solutionfile, debug, build_64_bit, concurrency, vspath, environ, clean):
        if debug and not build_64_bit:
            mode = "Debug|Win32"
        if not debug and not build_64_bit:
            mode = "Release|Win32"
        if debug and build_64_bit:
            mode = "Debug|x64"
        if not debug and build_64_bit:
            mode = "Release|x64"
        
        msdevExec = os.path.join(vspath, "vc", "vcpackages", "vcbuild.exe")
        arguments = [msdevExec, solutionfile]
        if clean:
            arguments.append("/clean")
        arguments.append("/error:[ERROR]")
        arguments.append("/warning:[WARNING]")
        arguments.append("/M%s" % (concurrency))
        arguments.append(mode)
        msdevProcess = subprocess.Popen(arguments,
                                        env=environ,
                                        stdin=subprocess.PIPE,
                                        stdout=sys.stdout)
        msdevProcess.stdin.close()
        retCode = msdevProcess.wait()
        if retCode != 0:
            raise ScriptException("Visual Studio did not compile project", retCode)
        
class Windows32bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug, visualstudio):
        WindowsBuilder.__init__(self, dependencies, arcsdk, build_in_debug, visualstudio)
        self.is_64_bit = False
        self.platform = "Win32"

class Windows64bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug, visualstudio):
        WindowsBuilder.__init__(self, dependencies, arcsdk, build_in_debug, visualstudio)
        self.is_64_bit = True
        self.platform = "x64"
        
class SolarisBuilder(Builder):
    def __init__(self, dependencies, arcsdk, build_in_debug):
        Builder.__init__(self, dependencies, arcsdk, build_in_debug)
    
    def getDoxygenPath(self):
        return os.path.join(self.depend_path, "doxygen", "bin", "doxygen")
        
    def compile_code(self, env, clean, build_opticks, concurrency):
        #Build Opticks Core
        self.run_scons(os.path.abspath("application"), self.build_debug_mode, concurrency, env, clean, ["core"])
        #Build PlugIns
        self.run_scons(os.path.abspath("application/PlugIns/src"), self.build_debug_mode, concurrency, env, clean, [])
        if build_opticks != "core":
            self.run_scons(os.path.abspath("application"), self.build_debug_mode, concurrency, env, clean, ["arcproxy"])
            
    def run_scons(self, path, debug, concurrency, environ, clean, extra_args=None):
        oldCurDir = os.getcwd()
        os.chdir(path)
        sconsExec = "scons"
        arguments = [sconsExec, "--no-cache"]
        arguments.append("-j%s" % (concurrency))
        if clean:
            arguments.append("-c")
        if not debug:
            arguments.append("RELEASE=yes")
        if extra_args:
            arguments.extend(extra_args)
        newnum = os.dup(sys.stdout.fileno())
        sconsProcess = subprocess.Popen(arguments,
                                        env=environ,
                                        stdin=subprocess.PIPE,
                                        stdout=newnum)
        sconsProcess.stdin.close() #Not sure if necessary on Solaris
        retCode = sconsProcess.wait()
        os.chdir(oldCurDir)
        if retCode != 0:
            raise ScriptException("Scons did not compile project", retCode)    

    def getBinariesDir(self, build_dir):
        return join(build_dir,"Binaries-solaris-sparc-%s"%(self.mode))

    def getZipName(self):
        return "Binaries-solaris-sparc-%s.zip"%(self.mode)

    def set_opticks_defaults(self, build_dir):
        config_defaults_dir = os.path.abspath(join(self.getBinariesDir(build_dir), "DefaultSettings"))
        if not os.path.exists(config_defaults_dir):
           os.makedirs(config_defaults_dir)
        plugin_path_defaults = join(config_defaults_dir, "100-PlugInPath-defaults.cfg")
        if not os.path.exists(plugin_path_defaults):
           shutil.copy2("PlugInPath-defaults.cfg", plugin_path_defaults) 

        plugin_path = os.path.abspath(join(self.getBinariesDir(build_dir),"PlugIns"))
        self.update_cfg_file(plugin_path_defaults, plugin_path) 

        return config_defaults_dir
        
    def prep_to_run(self, lib_path, default_dir, opticks_home, build_dir):
        dp = self.depend_path

        if build_dir == None:
            build_dir = "Build"

        so_path = "LibsFor%s" % (self.mode)
        so_path = os.path.abspath(join(lib_path, so_path))
            
        if not os.path.exists(so_path):
            os.makedirs(so_path)

        dp_list = commonutils.get_dependencies(self.depend_path, "Solaris", self.build_debug_mode, None) 
        commonutils.copy_dependencies(dp_list, so_path)

        qt_imageformat_plugins = ["libqgif.so", "libqjpeg.so", "libqmng.so", "libqsvg.so", "libqtiff.so"]
        plugin_dest_dir = os.path.abspath(join(self.getBinariesDir(build_dir),"Bin","imageformats"))
        if not os.path.exists(plugin_dest_dir):
            os.makedirs(plugin_dest_dir)
        for qt_plugin in qt_imageformat_plugins:
            plugin_src = os.path.abspath(join(self.depend_path,"Qt","plugins","solaris-sparc","imageformats",qt_plugin))
            shutil.copy2(plugin_src, os.path.abspath(join(plugin_dest_dir,qt_plugin)))        

        env = dict()
        env["LD_LIBRARY_PATH_64"] = so_path

        if opticks_home == None:
           opticks_home = "./Release"
           
        temp_dir = join(opticks_home, "Temp")
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)


        if build_dir == None:
           build_dir = "Build"

        app_setting_dir = join(build_dir, "ApplicationUserSettings")
        if not os.path.exists(app_setting_dir):
           os.makedirs(app_setting_dir)
        os.makedirs(temp_dir)

        if not default_dir:
           default_dir = self.set_opticks_defaults(build_dir)
            
        return (so_path, default_dir)
 
def create_builder(opticks_depends, arcsdk, build_in_debug, visualstudio, arch):
     builder = None
     if sys.platform.startswith("sunos"):
         builder = SolarisBuilder(opticks_depends, arcsdk, build_in_debug)
     elif sys.platform.startswith("win"):
         if arch == "32":
             builder = Windows32bitBuilder(opticks_depends, arcsdk, build_in_debug, visualstudio)
         if arch == "64":
             builder = Windows64bitBuilder(opticks_depends, arcsdk, build_in_debug, visualstudio)
     return builder

def prep_to_run(opticks_depends, build_debug, arch, visualstudio, lib_path, default_dir, opticks_home, build_dir):
    try:
        builder = create_builder(opticks_depends, False, build_debug, visualstudio, arch)
        if builder != None:
            return builder.prep_to_run(lib_path, default_dir, opticks_home, build_dir)
    except ScriptException, se:        
        pass
             
def print_env(environ):
    echo("Environment is currently set to")
    for key in environ.iterkeys():
        echo(key, "=", environ[key])

def echo(*list):
    for item in list:
        print item,
    print
    sys.stdout.flush()

def copy_files_in_dir(src_dir, dst_dir, with_extension=None):
    for filename in os.listdir(src_dir):
        full_path = join(src_dir, filename)
        if os.path.isfile(full_path):
            if with_extension == None or filename.endswith(with_extension):
                copy_file(src_dir, dst_dir, filename)

def copy_file(src_dir, dst_dir, filename):
    dst_file = os.path.join(dst_dir, filename)
    if os.path.exists(dst_file):
       os.remove(dst_file)
    shutil.copy2(os.path.join(src_dir, filename),
                 dst_file)
                
def main():
    options = optparse.OptionParser()
    options.add_option("-d", "--dependencies", dest="dependencies", action="store", type="string")
    options.add_option("-a", "--arcsdk", dest="arcsdk", action="store", type="string")
    vs_path = "C:\\Program Files (x86)\\Microsoft Visual Studio 8"
    if not os.path.exists(vs_path):
       vs_path = "C:\\Program Files\\Microsoft Visual Studio 8"
    options.add_option("--visualstudio", dest="visualstudio", action="store", type="string", default=vs_path)
    options.add_option("-m", "--mode", dest="mode", action="store", type="choice", choices=["debug", "release"], default="release", help="Use debug or release.")
    options.add_option("--arch", dest="arch", action="store", type="choice", choices=["32","64"], default="64", help="Use 32 or 64.")
    options.add_option("--clean", dest="clean", action="store_true", default=False)
    options.add_option("--build-opticks", dest="buildOpticks", action="store", type="choice", choices=["all","core","none"], default="none", help="Use all, core or none.")
    options.add_option("--update-version", dest="update_version_scheme", action="store", type="choice", choices=["milestone", "nightly", "none", "production", "rc", "unofficial"], default="none", help="Use milestone, nightly, production, rc, unofficial or none.  When using milestone, production, or rc you will need to use --new-version to provide the complete version # and --release-date to set the release date.  Using production will mark the application as production, all others will mark the application as not for production.  The unofficial and nightly will mutate the existing version #, so --new-version is not required.")
    options.add_option("--new-version", dest="new_version", action="store", type="string", default=None)
    options.add_option("--release-date", dest="release_date", action="store", type="string", default=None, help="Use YYYY-MM-DD or the special value, today")
    options.add_option("--prep",dest="prep",action="store_true",default=False)
    options.add_option("--dependent-libraries",dest="dependent_libraries_dir",action="store",default="Release")
    options.add_option("--defaultDir",dest="defaultDir",action="store",default=None)
    options.add_option("--opticksHome",dest="opticksHome",action="store",default=None)
    options.add_option("--artifact-dir",dest="artifactDir",action="store",default=None)
    options.add_option("--build-doxygen",dest="buildDoxygen",action="store_true",default=False)
    options.add_option("--concurrency",dest="concurrency",action="store",default=1)
    (options, args) = options.parse_args()

    builder = None
    try:
        opticks_depends = os.environ.get("OPTICKSDEPENDENCIES", None)
        
        if options.dependencies:
            #allow the -d command-line option to override environment variable
            opticks_depends = options.dependencies
        if not opticks_depends:
            #they did not use -d command-line option, nor is an environment variable
            #set so consider that an error        
            raise ScriptException("Dependencies argument must be provided", 1000)                
        if not os.path.exists(opticks_depends):
            raise ScriptException("Dependencies path is invalid", 1020)
            
        if options.mode == "debug":
            build_in_debug = True
        else:
            build_in_debug = False
        
        if sys.platform.startswith("win"):
            if not os.path.exists(options.visualstudio):
                raise ScriptException("Visual Studio path is invalid", 1010)
        
        builder = create_builder(opticks_depends, options.arcsdk, build_in_debug, options.visualstudio, options.arch)
        if builder == None:
           raise ScriptException("Unable to create builder for platform", 1030)

        builder.update_app_version_number(options.update_version_scheme, options.new_version, options.release_date)

        builder.build_executable(options.clean, options.buildOpticks, options.concurrency)
            
        if options.buildDoxygen:
            builder.build_doxygen(options.artifactDir)

        if options.prep:
            builder.prep_to_run(options.dependent_libraries_dir, options.defaultDir, options.opticksHome, None)
            
        if options.artifactDir:
            builder.gather_artifacts(options.artifactDir)

    except ScriptException, se:        
        print "--------------------------"
        if se.retcode >= 1000:
            print "SCRIPT ERROR:", se.msg
        else:
            print "BUILD ERROR:", se.msg
        traceback.print_exc()
        print "--------------------------"        
        return se.retcode
    except Exception, e:
        print "--------------------------"
        print "UNKNOWN ERROR:", e
        traceback.print_exc()
        print "--------------------------"
        return 2000;
                
    return 0

if __name__ == "__main__":
    retcode = main()
    print "Return code is", retcode
    sys.exit(retcode)
