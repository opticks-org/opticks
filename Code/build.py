#!/usr/bin/env python
import sys
import subprocess
import os
import os.path
from os.path import join
import optparse
import traceback
import shutil
import datetime
import time
import zipfile
import re
import codecs
import commonutils

def execute_process(args, bufsize=0, executable=None, preexec_fn=None,
      close_fds=None, shell=False, cwd=None, env=None,
      universal_newlines=False, startupinfo=None, creationflags=0):
    if sys.platform.startswith("win"):
        stdin = subprocess.PIPE
        stdout = sys.stdout
        stderr = sys.stderr
    else:
        stdin = None
        stdout = None
        stderr = None
    process = subprocess.Popen(args, bufsize=bufsize, stdin=stdin,
          stdout=stdout, stderr=stderr, executable=executable,
          preexec_fn=preexec_fn, close_fds=close_fds, shell=shell,
          cwd=cwd, env=env, universal_newlines=universal_newlines,
          startupinfo=startupinfo, creationflags=creationflags)
    if sys.platform.startswith("win"):
        process.stdin.close()
    returncode = process.wait()
    return returncode


def set_sln_startup_project(solution_file, project):
    # Read the sln file.
    file = open(solution_file, "r")
    lines = file.readlines()
    file.close()

    # Move the specified project to the beginning of the file.
    # Need to track the index due to duplicates in the list (i.e.: project dependencies).
    if (lines[0] == "Microsoft Visual Studio Solution File, Format Version 11.00\n"):
        begin = -1
        end = -1
        for i in range(len(lines)):
            line = lines[i]
            if (begin < 0 and line.find(project) >= 0):
               begin = i
            if (begin >= 0 and line == "EndProject\n"):
               end = i + 1
               break

    # Write the sln file to disk.
    if (begin >= 0 and end > begin):
        new_file = open(solution_file, "w")
        new_file.writelines(lines[0:2] + lines[begin:end] + lines[2:begin] + lines[end:])
        new_file.close()

class ScriptException(Exception):
    """Report error while running script"""

def is_windows():
    """Determine if this script is executing on the Windows operating system.
    @return: Return True if script is executed on Windows, False otherwise.
    @rtype: L{bool}

    """
    return sys.platform.startswith("win32")

class Builder:
    def __init__(self, dependencies, arcsdk, build_in_debug, verbosity):
        self.depend_path = dependencies
        self.arcsdk_path = arcsdk
        self.build_debug_mode = build_in_debug
        self.verbosity = verbosity
        if self.build_debug_mode:
            self.mode = "debug"
        else:
            self.mode = "release"
        self.initial_cmake_config = None
        self.make_program = "make"

    def get_app_version_only(self):
        return commonutils.get_app_version_only(".")

    def __get_build_revision_only(self):
        build_revision = "Unknown"
        build_revision_path = join("application",
            "Utilities", "BuildRevision.h")
        if not(os.path.exists(build_revision_path)):
            return build_revision
        build_revision_file = open(build_revision_path, "rt")
        revision_info = build_revision_file.read()
        build_revision_file.close()

        build_revision_match = re.search(r'BUILD_REVISION +?"(.*?)"',
            revision_info)
        if build_revision_match is not None:
            build_revision = build_revision_match.group(1)
        return build_revision

    def __update_h_file(self, h_file, fields_to_replace):
        h_handle = open(h_file, "rt")
        contents = h_handle.readlines()
        h_handle.close()
        h_handle = open(h_file, "wt")
        for vline in contents:
            fields = vline.strip().split()
            if len(fields) >= 3 and fields[0] == '#define' and \
                    fields[1] in fields_to_replace:
                h_handle.write('#define %s %s\n' % (fields[1],
                    fields_to_replace[fields[1]]))
            else:
                h_handle.write(vline)
        h_handle.close()

    def __update_app_version_h(self, fields_to_replace):
        app_version_path = join("application", "PlugInUtilities",
            "AppVersion.h")
        self.__update_h_file(app_version_path, fields_to_replace)

        #update modification time to force compilation to occur
        #even in incremental mode
        os.utime(join("application", "Utilities",
            "ConfigurationSettingsImp.cpp"), None)

    def __update_build_revision_h(self):
        #command_prefix = "./"
        #if is_windows():
        #    command_prefix = ""
        #execute_process(["python",
        #    "%supdate-build-revision.py" % (command_prefix)], shell=False)
        #### TODO: deal with this, probably just remove it since cmake handles this
        pass

    def get_current_app_version(self):
        # Try to update the build revision, since it might be stale
        self.__update_build_revision_h()

        # Read the app version directly from the file
        version_number = self.get_app_version_only()
        if version_number is not None:
            build_revision = self.__get_build_revision_only()
            # Concat the version number and build revision
            return version_number + " Build " + build_revision

        return "Unknown"

    def update_app_version_number(self, scheme, new_version, release_date):
        if scheme is None or scheme == "none":
            return

        if self.verbosity > 1:
            print "Updating app version..."

        # Read the app version directly from the file
        self.__update_build_revision_h()
        version_number = self.get_app_version_only()
        if version_number is None:
            raise ScriptException("Could not determine the "\
                "current app version while attempting to update "\
                "the app version")
        if self.verbosity >= 1: \
            print "Original version # of Opticks was", version_number
        version_number = commonutils.update_app_version(version_number,
            scheme, new_version, self.__get_build_revision_only())
        if self.verbosity >= 1:
            print "Setting version # of Opticks to", version_number

        # Update AppVersion.h
        fields_to_replace = dict()
        fields_to_replace["APP_VERSION_NUMBER"] = '"' + version_number + '"'
        if scheme == "production":
            fields_to_replace["APP_IS_PRODUCTION_RELEASE"] = "true"
            if self.verbosity >= 1:
                print "Making Opticks a production release"
        else:
            fields_to_replace["APP_IS_PRODUCTION_RELEASE"] = "false"
            if self.verbosity >= 1:
                print "Making Opticks a not for production release"

        date_obj = None
        if scheme == "nightly" or release_date == "today":
            date_obj = datetime.date.today()
        elif release_date is not None:
            if self.verbosity >= 1:
                print release_date
            date_obj = None
            try:
                date_tuple = time.strptime(release_date, "%Y-%m-%d")
                date_obj = datetime.date(*date_tuple[0:3])
            except Exception, e:
                print "ERROR: The release date is not in the proper "\
                    "format, use YYYY-MM-DD."

        if date_obj is not None:
            fields_to_replace["APP_RELEASE_DATE_YEAR"] = str(date_obj.year)
            fields_to_replace["APP_RELEASE_DATE_MONTH"] = str(date_obj.month)
            fields_to_replace["APP_RELEASE_DATE_DAY"] = str(date_obj.day)
            if self.verbosity >= 1:
                print "Updating the release date to %s" % \
                    (date_obj.isoformat())
        else:
            if self.verbosity >= 1:
                print "The release date has not been updated"

        self.__update_app_version_h(fields_to_replace)

        opticks_version_fields = dict()
        opticks_version_fields["OPTICKS_VERSION"] = fields_to_replace["APP_VERSION_NUMBER"]
        self.__update_h_file(join("application", "Interfaces", "OpticksVersion.h"),
            opticks_version_fields)

        if self.verbosity > 1:
            print "Done updating app version"

    def populate_environ_for_dependencies(self, env):
        env["OPTICKSDEPENDENCIES"] = self.depend_path
        if self.arcsdk_path:
            env["ARCSDK"] = self.arcsdk_path

    def run_cmake(self, debug, environ, extra_args=None):
        path = self.get_build_path()
        if self.verbosity > 1:
            print "Determining if build is configured..."
        if self.check_if_configured(path):
            # already configured...the build will determine if cmake needs to run again
            return
        if self.verbosity > 1:
            print "Configuring build with cmake..."
        arguments = ["cmake", "-G", self.generator_name]
        if debug:
            arguments.append("-DCMAKE_BUILD_TYPE=debug")
        else:
            arguments.append("-DCMAKE_BUILD_TYPE=release")
        if self.initial_cmake_config:
            arguments.extend(["-C",self.initial_cmake_config])
        if extra_args is not None:
            arguments.extend(extra_args)
        arguments.append(os.path.abspath("application"))
        retcode = execute_process(arguments, cwd=path, env=environ)
        if retcode != 0:
            raise ScriptException("CMake configuration failed")

    def run_make(self, environ, target=None, concurrency=None, extra_args=None):
        path = self.get_build_path()
        arguments = [self.make_program]
        if concurrency is not None:
            arguments.append("-j%s" % concurrency)
        if extra_args is not None:
            arguments.extend(extra_args)
        if target is not None:
            arguments.append(target)
        retcode = execute_process(arguments, cwd=path, env=environ)
        if retcode != 0:
            raise ScriptException("Make did not compile project")

    def build_executable(self, clean_build_first, build_opticks, concurrency):
        #No return code, throw exception or ScriptException
        if build_opticks == "none":
            return

        if self.verbosity > 1:
            print "Building executable..."
        buildenv = os.environ
        self.populate_environ_for_dependencies(buildenv)

        if build_opticks in ["arcproxy", "all"]:
            if not "ARCSDK" in buildenv:
                raise ScriptException("ARCSDK is not provided")
            if not os.path.exists(buildenv.get("ARCSDK")):
                raise ScriptException("ArcSDK path does not exist")

        if self.verbosity >= 1:
            print_env(buildenv)
        if build_opticks in ["core","all"]:
           if self.verbosity > 1:
               print "Configuring with cmake..."
           self.run_cmake(self.build_debug_mode, buildenv)

           if clean_build_first:
               if self.verbosity > 1:
                   print "Cleaning compilation..."
               self.compile_code(buildenv, True, build_opticks, concurrency)
               if self.verbosity > 1:
                   print "Done cleaning compilation"

           self.compile_code(buildenv, False, build_opticks, concurrency)
           if self.verbosity > 1:
               print "Done building executable"

    def gather_artifacts(self, artifacts_dir):
        binaries_dir = os.path.abspath(self.get_binaries_dir(self.get_build_path()))
        zip_name = self.get_zip_name()
        zip_path = os.path.abspath(join(artifacts_dir, zip_name))
        if self.verbosity > 1:
            print "Gathering Build artifacts from %s into %s.." % \
                (binaries_dir, zip_path)
        the_zip = zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED)
        for cur_dir, dirs, files in os.walk(binaries_dir):
            root = cur_dir[len(binaries_dir):]
            if os.path.isabs(root):
                root = apply(join, root.split(os.path.sep)[1:])
            for the_file in files:
                the_zip.write(join(cur_dir, the_file), join(root, the_file))

        the_zip.close()
        if self.verbosity > 1:
            print "Done gathering Build artifacts"

    def update_testbed_cfg_file(self, cfgfile, test_data_path):
        def importpath_repl(matchobj):
            return test_data_path
        # Update the import path setting in the .cfg file
        # to the est data location
        cfgfileHandle = codecs.open(cfgfile, "r", "utf-8")
        contents = cfgfileHandle.read()
        cfgfileHandle.close()
        contents = re.sub("TESTDATAPATH", importpath_repl, contents)
        cfgfileHandle = codecs.open(cfgfile, "w+", "utf-8")
        cfgfileHandle.write(contents)
        cfgfileHandle.close()

    def generic_prep_to_run(self, build_dir, os_spec, arch):
        bin_path = os.path.abspath(join(self.get_binaries_dir(build_dir),
            "Bin"))

        if not os.path.exists(bin_path):
            os.makedirs(bin_path)

        if self.verbosity > 1:
            print "Gathering dependency libraries..."
        dp_list = commonutils.get_dependencies(self.depend_path,
            os_spec, self.build_debug_mode, arch)
        commonutils.copy_dependencies(dp_list, bin_path)
        if self.verbosity > 1:
            print "Done gathering dependency libraries"

        temp_dir = join("Release", "Temp")
        if os.path.exists(temp_dir):
            if self.verbosity > 1:
                print "Removing Temp directory folder at %s..." % (temp_dir)
            shutil.rmtree(temp_dir)
            if self.verbosity > 1:
                print "Done removing Temp directory"

        if build_dir is None:
            build_dir = self.get_build_path()

        app_setting_dir = os.path.abspath(join(build_dir,
            "ApplicationUserSettings"))
        if not os.path.exists(app_setting_dir):
            if self.verbosity > 1:
                print "Creating ApplicationUserSettings folder at %s..." % \
                    (app_setting_dir)
            os.makedirs(app_setting_dir)
            if self.verbosity > 1:
                print "Done creating ApplicationUserSettings folder"
        if self.verbosity > 1:
            print "Creating Temp directory folder at %s..." % (temp_dir)
        os.makedirs(temp_dir)
        if self.verbosity > 1:
            print "Done creating Temp directory folder"

        testbed_settings_dir = os.path.abspath(join(build_dir, "TestBedUserSettings"))
        if not os.path.exists(testbed_settings_dir):
            if self.verbosity > 1:
                print "Creating TestBedUserSettings folder at %s..." % testbed_settings_dir
            os.makedirs(testbed_settings_dir)
            if self.verbosity > 1:
                print "Done creating TestBedUserSettings folder"

        defaults_dir = os.path.abspath(join(build_dir, "TestBedDefaultSettings"))
        if not os.path.exists(defaults_dir):
            if self.verbosity > 1:
                print "Creating TestBedDefaultSettings folder at %s..." % defaults_dir
            os.makedirs(defaults_dir)
            if self.verbosity > 1:
                print "Done creating TestBedDefaultsSettings folder"

        if self.verbosity > 1:
            print "Creating TestBed defaults config file..."
        shutil.copy2(os.path.abspath("TestBed-defaults.cfg"),
            join(defaults_dir, "50-TestBed-defaults.cfg"))
        self.update_testbed_cfg_file(join(defaults_dir, "50-TestBed-defaults.cfg"), self.test_data_path)
        if self.verbosity > 1:
            print "Done creating TestBed defaults config file..."

        dep_file_path = join(bin_path, "opticks.dep")
        if not(os.path.exists(dep_file_path)):
            if self.verbosity > 1:
                print "Creating opticks.dep file..."
            dep_file = open(dep_file_path, "w")
            dep_file.write("!depV1 { deployment: { "\
                "AppHomePath: ../../../Release, "\
                "UserConfigPath: ../../ApplicationUserSettings, "\
                "PlugInPath: ../plugins } }")
            dep_file.close()
            if self.verbosity > 1:
                print "Done creating opticks.dep file"

        dep_file_path = join(bin_path, "testBed.dep")
        if not(os.path.exists(dep_file_path)):
            if self.verbosity > 1:
                print "Creating testBed.dep file..."
            dep_file = open(dep_file_path, "w")
            dep_file.write("!depV1 { deployment: { "\
                "AppHomePath: ../../../Release, "\
                "UserConfigPath: ../../TestBedUserSettings, "\
                "AdditionalDefaultPath: "\
                " ../../TestBedDefaultSettings } }")
            dep_file.close()
            if self.verbosity > 1:
                print "Done creating testBed.dep file"

        return bin_path

class WindowsBuilder(Builder):
    def __init__(self, dependencies, arcsdk, build_in_debug,
                 verbosity, msbuild, ms_help_compiler, test_data_path):
        Builder.__init__(self, dependencies, arcsdk, build_in_debug, verbosity)
        self.msbuild_path = msbuild 
        self.ms_help_compiler = ms_help_compiler
        if test_data_path is not None:
            self.test_data_path = test_data_path
        else:
            self.test_data_path = "T:/cppTestData"

    def run_cmake(self, debug, environ, extra_args=None):
        # Run the generic process then modify the generated Opticks.sln to force Opticks to be the startup project.
        solution_file = os.path.join(self.get_build_path(), "Opticks.sln")
        new_sln = not os.path.exists(solution_file)
        Builder.run_cmake(self, debug, environ, extra_args)
        if new_sln:
            if (self.verbosity > 1):
                print "Setting the sln file startup project."
            set_sln_startup_project(solution_file, "Opticks.vcxproj")

    def compile_code(self, env, clean, build_opticks, concurrency):
        if build_opticks in ["core","all"]:
           if self.verbosity > 1:
               print "Building Opticks and plug-ins"
           solution_file = os.path.join(self.get_build_path(), "Opticks.sln")
           if clean:
               self.build_in_msbuild(solution_file,
                   self.build_debug_mode, self.is_64_bit, concurrency,
                   self.msbuild_path, env, target="clean")
           else:
               self.build_in_msbuild(solution_file,
                   self.build_debug_mode, self.is_64_bit, concurrency,
                   self.msbuild_path, env)
           if self.verbosity > 1:
               print "Done building Opticks and plug-ins"
        if build_opticks in ["arcproxy","all"]:
            if self.verbosity > 1:
                print "Building ArcProxy"
            # This sln is separate from Opticks.sln for two reasons:
            #    1. ArcProxy is only built for 32-bit
            #       - The 64-bit parts of the sln are just to put it into the right build directory.
            #    2. ArcProxy uses the #import directive, which is incompatible with the /MP flag.
            #       - See http://msdn.microsoft.com/en-us/library/bb385193.aspx
            solution_file = os.path.join(self.get_build_path(), "../application/ArcIntegration.sln")
            if clean:
                self.build_in_msbuild(solution_file,
                    self.build_debug_mode, self.is_64_bit,
                    concurrency, self.msbuild_path, env, target="clean")
            else:
                self.build_in_msbuild(solution_file,
                    self.build_debug_mode, self.is_64_bit,
                    concurrency, self.msbuild_path, env)
            if self.verbosity > 1:
                print "Done building ArcProxy"

    def get_binaries_dir(self, build_dir):
        return join(get_build_path(),"Binaries")

    def get_zip_name(self):
        return "Binaries-%s-%s.zip" % (self.platform, self.mode)

    def get_doxygen_path(self):
        doxygen_path = join(self.depend_path, "32", "bin", "doxygen.exe")
        if os.path.exists(doxygen_path):
            return doxygen_path
        doxygen_path = join(self.depend_path, "64", "bin", "doxygen.exe")
        return doxygen_path

    def other_doxygen_prep(self, build, env):
        if build != "all":
            return
        if self.verbosity > 1:
            print "Enabling CHM generation"
        hhc_path = os.path.abspath(join(self.ms_help_compiler, "hhc.exe"))
        if not(os.path.exists(hhc_path)):
            raise ScriptException("MS Help Compiler path of %s is "\
                "invalid, see --ms-help-compiler" %
                (self.ms_help_compiler))
        env["GENERATE_CHM"] = "YES"
        env["MICROSOFT_HELP_COMPILER"] = hhc_path
        chm_file = "OpticksSDK-%s.chm" % (self.get_app_version_only())
        env["CHM_NAME"] = chm_file

    def check_if_configured(self, path):
        if os.path.exists(os.path.join(path, "Opticks.sln")):
            return True
        if not os.path.exists(path):
            os.makedirs(path)
        return False

    def prep_to_run(self, build_dir):
        if build_dir is None:
            build_dir = self.get_build_path()

        bin_path = self.generic_prep_to_run(build_dir, "Windows",
            "64" if self.is_64_bit else "32")

        # Qt DLLs for ArcProxy
        if self.verbosity > 1:
            print "Gathering Qt dll's needed for ArcProxy..."
        proxy_dir = join(self.get_binaries_dir(build_dir),
            "PlugIns", "ArcProxy")
        if not os.path.exists(proxy_dir):
            os.makedirs(proxy_dir)

        proxy_dll_names = []
        if self.build_debug_mode:
            proxy_dll_names = ["QtCored4.dll", "QtNetworkd4.dll"]
        else:
            proxy_dll_names = ["QtCore4.dll", "QtNetwork4.dll"]
        for proxy_dll_name in proxy_dll_names:
            shutil.copy2(join(self.depend_path, "32",
                "lib", proxy_dll_name),
                join(proxy_dir, proxy_dll_name))
        if self.verbosity > 1:
            print "Done gathering Qt dll's needed for ArcProxy"

        return bin_path

    def build_in_msbuild(self, solutionfile, debug,
                         build_64_bit, concurrency,
                         msbuildpath, environ, target=None):
        if not os.path.exists(msbuildpath):
            raise ScriptException("MS Build path is invalid")
    
        if debug:
            config = "Debug"
        else:
            config = "Release"
        if build_64_bit:
            platform = "x64"
        else:
            platform = "Win32"

        msbuild_exec = join(msbuildpath, "msbuild.exe")
        arguments = [msbuild_exec, solutionfile]
        if target is not None:
            arguments.append("/target:%s" % target)
        arguments.append("/m:%s" % concurrency)
        arguments.append("/p:Platform=%s" % platform)
        arguments.append("/p:Configuration=%s" % config)
        retcode = execute_process(arguments, env=environ)
        if retcode != 0:
            raise ScriptException("Visual Studio did not compile project")

class Windows32bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug,
                 verbosity, msbuild, ms_help_compiler, test_data_path):
        WindowsBuilder.__init__(self, dependencies, arcsdk,
            build_in_debug, verbosity, msbuild, ms_help_compiler, test_data_path)
        self.is_64_bit = False
        self.platform = "Win32"
        self.generator_name = 'Visual Studio 10'

    def get_build_path(self):
        return os.path.abspath("Build32")

class Windows64bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug,
                 verbosity, msbuild, ms_help_compiler, test_data_path):
        WindowsBuilder.__init__(self, dependencies, arcsdk,
            build_in_debug, verbosity, msbuild, ms_help_compiler, test_data_path)
        self.is_64_bit = True
        self.platform = "x64"
        self.generator_name = 'Visual Studio 10 Win64'

    def get_build_path(self):
        return os.path.abspath("Build64")

class SolarisBuilder(Builder):
    def __init__(self, dependencies, arcsdk, build_in_debug, verbosity, test_data_path):
        Builder.__init__(self, dependencies, arcsdk, build_in_debug, verbosity)
        self.generator_name = 'Unix Makefiles'
        self.initial_cmake_config = os.path.abspath("application/SunOS-64-cmake-cache.txt")
        self.make_program = "gmake"
        if test_data_path is not None:
            self.test_data_path = test_data_path
        else:
            self.test_data_path = "/TestData/cppTestData"

    def get_build_path(self):
        if self.build_debug_mode:
           return os.path.abspath("build-debug")
        return os.path.abspath("build-release")

    def get_doxygen_path(self):
        return join(self.depend_path, "64", "bin", "doxygen")

    def other_doxygen_prep(self, build, env):
        graphviz_dir = os.path.abspath(join(self.depend_path,
            "graphviz", "app"))
        env["GVBINDIR"] = join(graphviz_dir, "lib", "graphviz")
        new_value = join(graphviz_dir, "lib")
        if env.has_key("LD_LIBRARY_PATH_32"):
            new_value = new_value + ":" + env["LD_LIBRARY_PATH_32"]
        env["LD_LIBRARY_PATH_32"] = new_value

    def check_if_configured(self, path):
        if os.path.exists(os.path.join(path, "Makefile")):
            return True
        if not os.path.exists(path):
            os.makedirs(path)
        return False

    def compile_code(self, env, clean, build_opticks, concurrency):
        #Build Opticks Core
        if build_opticks in ["core","all"]:
           if clean:
               if self.verbosity > 1:
                   print "Cleaning build directory"
               self.run_make(env, target="clean", concurrency=concurrency)
           if self.verbosity > 1:
               print "Building Opticks Core"
           self.run_make(env, concurrency=concurrency)
           if self.verbosity > 1:
               print "Done building Opticks Core"
        if build_opticks in ["arcproxy","all"]:
            if self.verbosity > 1:
                print "Building ArcProxy"
            self.run_make(env, target="ArcProxy", concurrency=concurrency)
            if self.verbosity > 1:
                print "Done building ArcProxy"

    def get_binaries_dir(self, build_dir):
        return join(self.get_build_path(), "Binaries")

    def get_zip_name(self):
        return "Binaries-solaris-sparc-%s.zip" % (self.mode)

    def prep_to_run(self, build_dir):
        if build_dir is None:
            build_dir = self.get_build_path()
 
        return self.generic_prep_to_run(build_dir, "Solaris", "64")

class LinuxBuilder(SolarisBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug, verbosity, test_data_path):
        SolarisBuilder.__init__(self, dependencies, arcsdk, build_in_debug, verbosity, test_data_path)
        self.initial_cmake_config = None
        self.make_program = "make"

    def other_doxygen_prep(self, build, env):
        graphviz_dir = os.path.abspath(join("/", "usr", "bin"))
        env["DOT_DIR"] = join(graphviz_dir)
        env["GVBINDIR"] = join("/", "usr", "lib", "graphviz")
        new_value = join(graphviz_dir, "lib")
        if env.has_key("LD_LIBRARY_PATH"):
            new_value = new_value + ":" + env["LD_LIBRARY_PATH"]
        env["LD_LIBRARY_PATH"] = new_value

    def get_doxygen_path(self):
        return join("/","usr","bin","doxygen")

    def get_zip_name(self):
        return "Binaries-linux-x86_64-%s.zip" % (self.mode)

    def prep_to_run(self, build_dir):
        if build_dir is None:
            build_dir = self.get_build_path()

        return self.generic_prep_to_run(build_dir, "Linux", "64")

def create_builder(opticks_depends, arcsdk, build_in_debug,
                   msbuild, ms_help_compiler, arch, verbosity, test_data_path):
    builder = None
    if is_windows():
        if arch == "32":
            builder = Windows32bitBuilder(opticks_depends, arcsdk,
                build_in_debug, verbosity, msbuild, ms_help_compiler,
                test_data_path)
        if arch == "64":
            builder = Windows64bitBuilder(opticks_depends, arcsdk,
                build_in_debug, verbosity, msbuild, ms_help_compiler,
                test_data_path)
    elif sys.platform.startswith("linux"):
        builder = LinuxBuilder(opticks_depends, arcsdk, build_in_debug, verbosity,
            test_data_path)
    else:
        builder = SolarisBuilder(opticks_depends, arcsdk,
            build_in_debug, verbosity, test_data_path)
    return builder

def print_env(environ):
    print "Environment is currently set to"
    for key in environ.iterkeys():
        print key, "=", environ[key]

def copy_files_in_dir(src_dir, dst_dir, with_extension=None):
    for filename in os.listdir(src_dir):
        full_path = join(src_dir, filename)
        if os.path.isfile(full_path):
            if with_extension is None or filename.endswith(with_extension):
                copy_file(src_dir, dst_dir, filename)

def copy_file(src_dir, dst_dir, filename):
    dst_file = join(dst_dir, filename)
    if os.path.exists(dst_file):
        os.remove(dst_file)
    shutil.copy2(join(src_dir, filename),
                 dst_file)

def main(args):
    #chdir to the directory where the script resides
    os.chdir(os.path.abspath(os.path.dirname(sys.argv[0])))

    options = optparse.OptionParser()
    options.add_option("-d", "--dependencies",
        dest="dependencies", action="store", type="string")
    options.add_option("-a", "--arcsdk", dest="arcsdk",
        action="store", type="string")
    if is_windows():
        msbuild_path = "C:\\Windows\\Microsoft.NET\Framework\\v4.0.30319"
        options.add_option("--msbuild", dest="msbuild",
            action="store", type="string")
        ms_help_compiler_path = "C:\\Program Files (x86)\\HTML Help Workshop"
        if not os.path.exists(ms_help_compiler_path):
            ms_help_compiler_path = "C:\\Program Files\\HTML Help Workshop"
        options.add_option("--ms-help-compiler",
            dest="ms_help_compiler", action="store", type="string")
        options.add_option("--arch", dest="arch", action="store",
            type="choice", choices=["32","64"], help="Use 32 or 64.")
        options.set_defaults(msbuild=msbuild_path,
            ms_help_compiler=ms_help_compiler_path, arch="64")
    options.add_option("-m", "--mode", dest="mode",
        action="store", type="choice", choices=["debug", "release"],
        help="Use debug or release.")
    options.add_option("--clean", dest="clean", action="store_true")
    options.add_option("--build-opticks", dest="build_opticks",
        action="store", type="choice", choices=["all","arcproxy","core","none"],
        help="Use all, arcproxy, core or none.")
    options.add_option("--update-version", dest="update_version_scheme",
        action="store", type="choice",
        choices=["milestone", "nightly", "none", "production",
                 "rc", "unofficial"],
        help="Use milestone, nightly, production, rc, unofficial or "\
             "none.  When using milestone, production, or rc you will "\
             "need to use --new-version to provide the complete "\
             "version # and --release-date to set the release "\
             "date.  Using production will mark the application "\
             "as production, all others will mark the application "\
             "as not for production.  The unofficial and nightly "\
             "will mutate the existing version #, so --new-version is "\
             "not required.")
    options.add_option("--new-version", dest="new_version",
        action="store", type="string")
    options.add_option("--release-date", dest="release_date",
        action="store", type="string",
        help="Use YYYY-MM-DD or the special value, today")
    options.add_option("--prep", dest="prep", action="store_true")
    options.add_option("--artifact-dir", dest="artifact_dir", action="store")
    options.add_option("--test-data-path", dest="test_data_path", action="store")
    options.add_option("--concurrency", dest="concurrency", action="store")
    options.add_option("-q", "--quiet", help="Print fewer messages",
        action="store_const",dest="verbosity", const=0)
    options.add_option("-v", "--verbose", help="Print more messages",
        action="store_const", dest="verbosity", const=2)
    options.set_defaults(mode="release",
       clean=False, build_opticks="none", update_version_scheme="none",
       prep=False, test_data_path=None, concurrency=1, verbosity=1)

    options = options.parse_args(args[1:])[0]
    if not is_windows():
        options.msbuild = None
        options.ms_help_compiler = None
        options.arch = "64"

    builder = None
    try:
        opticks_depends = os.environ.get("OPTICKSDEPENDENCIES", None)

        if options.dependencies:
            #allow the -d command-line option to override environment variable
            opticks_depends = options.dependencies
        if not opticks_depends:
            if os.path.exists(os.path.abspath("../Dependencies")):
                opticks_depends = os.path.abspath("../Dependencies")
                print "Using %s for dependencies" % opticks_depends
        if not opticks_depends:
            #didn't use -d option, nor is an environment variable
            #set so consider that an error
            raise ScriptException("Dependencies argument must be provided")
        if not os.path.exists(opticks_depends):
            raise ScriptException("Dependencies path is invalid")

        if options.mode == "debug":
            build_in_debug = True
        else:
            build_in_debug = False

        builder = create_builder(opticks_depends, options.arcsdk,
            build_in_debug, options.msbuild,
            options.ms_help_compiler, options.arch, options.verbosity,
            options.test_data_path)
        if builder is None:
            raise ScriptException("Unable to create builder for platform")

        builder.update_app_version_number(options.update_version_scheme,
            options.new_version, options.release_date)

        if options.build_opticks != "none":
            #Force BuildRevision.h to be up-to-date, after AppVersion.h
            #might have been changed but before we do anything else.
            builder.get_current_app_version()

        builder.build_executable(options.clean, options.build_opticks,
            options.concurrency)

        if options.prep:
            if options.verbosity > 1:
                print "Prepping to Run..."
            builder.prep_to_run(None)
            if options.verbosity > 1:
                print "Done prepping to run"

        if options.artifact_dir:
            if options.verbosity > 1:
                print "Gathering artifacts"
            builder.gather_artifacts(options.artifact_dir)
            if options.verbosity > 1:
                print "Done gathering artifacts"

    except Exception, e:
        print "--------------------------"
        traceback.print_exc()
        print "--------------------------"
        return 2000

    return 0

if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
    retcode = main(sys.argv)
    print "Return code is", retcode
    sys.exit(retcode)
