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

    def build_doxygen(self, build, artifacts_dir):
        if self.verbosity > 1:
            print "Generating HTML..."
        current_app_version = self.get_current_app_version()
        doc_path = os.path.abspath(join("Build", "DoxygenOutput"))
        if os.path.exists(doc_path):
            #delete any already generated documentation
            shutil.rmtree(doc_path, True)
        os.makedirs(doc_path)
        doxygen_cmd = self.get_doxygen_path()
        config_dir = os.path.abspath(join("application", "ApiDocs"))
        args = [doxygen_cmd, join(config_dir, "application.dox")]
        env = os.environ
        env["SOURCE"] = os.path.abspath("application")
        env["VERSION"] = current_app_version
        env["OUTPUT_DIR"] = doc_path
        env["CONFIG_DIR"] = config_dir
        if is_windows():
            dot_exe = "dot.exe"
        else:
            dot_exe = "dot"
        graphviz_dir = os.path.abspath(join(self.depend_path,
            "64", "tools", "graphviz", "bin"))
        if not(os.path.exists(join(graphviz_dir, dot_exe))):
            graphviz_dir = os.path.abspath(join(self.depend_path,
                "32", "tools", "graphviz", "bin"))
        env["DOT_DIR"] = graphviz_dir
        self.other_doxygen_prep(build, env)
        retcode = execute_process(args, env=env)
        if retcode != 0:
            raise ScriptException("Unable to run doxygen generation script")
        if self.verbosity > 1:
            print "Done generating HTML"
        if artifacts_dir is not None:
            if self.verbosity > 1:
                print "Compressing Doxygen because --artifact-dir "\
                    "was provided."
            html_path = join(doc_path, "html")
            zip_name = "doxygen.zip"
            zip_path = os.path.abspath(join(artifacts_dir, zip_name))
            the_zip = zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED)
            for cur_dir, dirs, files in os.walk(html_path):
                arc_dir = cur_dir[len(html_path):]
                for the_file in files:
                    the_zip.write(join(cur_dir, the_file),
                        join(arc_dir,the_file))
            the_zip.close()
            if self.verbosity > 1:
                print "Done compressing Doxygen"

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
        command_prefix = "./"
        if is_windows():
            command_prefix = ""
        execute_process(["python",
            "%supdate-build-revision.py" % (command_prefix)], shell=False)

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

    def run_scons(self, path, debug, concurrency, environ,
                  clean, extra_args=None):
        scons_exec = "scons"
        if is_windows():
            scons_exec = scons_exec + ".bat"
        arguments = [scons_exec]
        arguments.append("-j%s" % (concurrency))
        if clean:
            arguments.append("-c")
        if not debug:
            arguments.append("RELEASE=yes")
        if extra_args:
            arguments.extend(extra_args)
        retcode = execute_process(arguments, cwd=path, env=environ)
        if retcode != 0:
            raise ScriptException("Scons did not compile project")

    def build_executable(self, clean_build_first, build_opticks, concurrency):
        #No return code, throw exception or ScriptException
        if build_opticks == "none":
            return

        if self.verbosity > 1:
            print "Building executable..."
        buildenv = os.environ
        self.populate_environ_for_dependencies(buildenv)

        if build_opticks != "core":
            if not "ARCSDK" in buildenv:
                raise ScriptException("ARCSDK is not provided")
            if not os.path.exists(buildenv.get("ARCSDK")):
                raise ScriptException("ArcSDK path does not exist")

        if self.verbosity >= 1:
            print_env(buildenv)

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
        binaries_dir = os.path.abspath(self.get_binaries_dir("Build"))
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
            build_dir = "Build"

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

        dep_file_path = join(bin_path, "opticks.dep")
        if not(os.path.exists(dep_file_path)):
            if self.verbosity > 1:
                print "Creating opticks.dep file..."
            dep_file = open(dep_file_path, "w")
            dep_file.write("!depV1 { deployment: { "\
                "AppHomePath: ../../../Release, "\
                "UserConfigPath: ../../ApplicationUserSettings } }")
            dep_file.close()
            if self.verbosity > 1:
                print "Done creating opticks.dep file"

        return bin_path

class WindowsBuilder(Builder):
    def __init__(self, dependencies, arcsdk, build_in_debug,
                 verbosity, msbuild, use_scons, ms_help_compiler):
        Builder.__init__(self, dependencies, arcsdk, build_in_debug, verbosity)
        self.msbuild_path = msbuild 
        self.ms_help_compiler = ms_help_compiler
        self.use_scons = use_scons

    def compile_code(self, env, clean, build_opticks, concurrency):
        if self.use_scons:
            if self.verbosity > 1:
                print "Building Opticks"

            extra_args = ["all"]
            arch_args = []
            if self.is_64_bit:
                arch_args.append("BITS=64")
            else:
                arch_args.append("BITS=32")
            self.run_scons(os.path.abspath("application"),
               self.build_debug_mode, concurrency, env, clean, extra_args+arch_args)
            if self.verbosity > 1:
                print "Done building Opticks"
                print "Building Opticks plug-ins"
            self.run_scons(os.path.abspath(r"application\PlugIns\src"),
               self.build_debug_mode, concurrency, env, clean, arch_args)
            if self.verbosity > 1:
                print "Done building Opticks plug-ins"
            if build_opticks != "core":
               if self.verbosity > 1:
                   print "Building ArcProxy"
               self.run_scons(os.path.abspath("application"),
                   self.build_debug_mode, "1", env, clean, ["arcProxy", "BITS=32"])
               if self.is_64_bit:
                   self.run_scons(os.path.abspath("application"),
                       self.build_debug_mode, "1", env, clean, ["arcProxy", "BITS=64"])
               if self.verbosity > 1:
                   print "Done building ArcProxy"
        else:
            if self.verbosity > 1:
                print "Building Opticks and plug-ins"
            solution_file = os.path.abspath("Application\\Opticks.sln")
            self.build_in_msbuild(solution_file,
                self.build_debug_mode, self.is_64_bit, concurrency,
                self.msbuild_path, env, clean)
            if self.verbosity > 1:
                print "Done building Opticks and plug-ins"
            if build_opticks != "core":
                if self.verbosity > 1:
                    print "Building ArcProxy"
                solution_file = os.path.abspath("Application\\ArcIntegration.sln")
                self.build_in_msbuild(solution_file,
                    self.build_debug_mode, self.is_64_bit,
                    concurrency, self.msbuild_path, env, clean)
                if self.verbosity > 1:
                    print "Done building ArcProxy"

    def get_binaries_dir(self, build_dir):
        return join(build_dir,"Binaries-%s-%s" % (self.platform, self.mode))

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

    def prep_to_run(self, build_dir):
        if build_dir is None:
            build_dir = "Build"

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
                         msbuildpath, environ, clean):
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
        if clean:
            arguments.append("/target:clean")
        arguments.append("/m:%s" % concurrency)
        arguments.append("/p:Platform=%s" % platform)
        arguments.append("/p:Configuration=%s" % config)
        retcode = execute_process(arguments, env=environ)
        if retcode != 0:
            raise ScriptException("Visual Studio did not compile project")

class Windows32bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug,
                 verbosity, msbuild, use_scons, ms_help_compiler):
        WindowsBuilder.__init__(self, dependencies, arcsdk,
            build_in_debug, verbosity, msbuild, use_scons, ms_help_compiler)
        self.is_64_bit = False
        self.platform = "Win32"

class Windows64bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug,
                 verbosity, msbuild, use_scons, ms_help_compiler):
        WindowsBuilder.__init__(self, dependencies, arcsdk,
            build_in_debug, verbosity, msbuild, use_scons, ms_help_compiler)
        self.is_64_bit = True
        self.platform = "x64"

class SolarisBuilder(Builder):
    def __init__(self, dependencies, arcsdk, build_in_debug, verbosity):
        Builder.__init__(self, dependencies, arcsdk, build_in_debug, verbosity)

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

    def compile_code(self, env, clean, build_opticks, concurrency):
        #Build Opticks Core
        if self.verbosity > 1:
            print "Building Opticks Core"
        self.run_scons(os.path.abspath("application"),
            self.build_debug_mode, concurrency, env, clean, ["core"])
        if self.verbosity > 1:
            print "Done building Opticks Core"
        #Build PlugIns
        if self.verbosity > 1:
            print "Building Opticks PlugIns"
        self.run_scons(os.path.abspath("application/PlugIns/src"),
            self.build_debug_mode, concurrency, env, clean, [])
        if self.verbosity > 1:
            print "Done building Opticks PlugIns"
        if build_opticks != "core":
            if self.verbosity > 1:
                print "Building ArcProxy"
            self.run_scons(os.path.abspath("application"),
                self.build_debug_mode, concurrency, env, clean, ["arcproxy"])
            if self.verbosity > 1:
                print "Done building ArcProxy"

    def get_binaries_dir(self, build_dir):
        return join(build_dir,"Binaries-solaris-sparc-%s" % (self.mode))

    def get_zip_name(self):
        return "Binaries-solaris-sparc-%s.zip" % (self.mode)

    def prep_to_run(self, build_dir):
        if build_dir is None:
            build_dir = "Build"

        return self.generic_prep_to_run(build_dir, "Solaris", "64")

class LinuxBuilder(SolarisBuilder):
    def __init__(self, dependencies, arcsdk, build_in_debug, verbosity):
        SolarisBuilder.__init__(self, dependencies, arcsdk, build_in_debug, verbosity)

    def other_doxygen_prep(self, build, env):
        graphviz_dir = os.path.abspath(join(self.depend_path,
            "graphviz", "app"))
        env["GVBINDIR"] = join(graphviz_dir, "lib", "graphviz")
        new_value = join(graphviz_dir, "lib")
        if env.has_key("LD_LIBRARY_PATH"):
            new_value = new_value + ":" + env["LD_LIBRARY_PATH"]
        env["LD_LIBRARY_PATH"] = new_value

    def get_binaries_dir(self, build_dir):
        return join(build_dir,"Binaries-linux-x86_64-%s" % (self.mode))

    def get_zip_name(self):
        return "Binaries-linux-x86_64-%s.zip" % (self.mode)

    def prep_to_run(self, build_dir):
        if build_dir is None:
            build_dir = "Build"

        return self.generic_prep_to_run(build_dir, "Linux", "64")

def create_builder(opticks_depends, arcsdk, build_in_debug,
                   msbuild, use_scons, ms_help_compiler, arch, verbosity):
    builder = None
    if is_windows():
        if arch == "32":
            builder = Windows32bitBuilder(opticks_depends, arcsdk,
                build_in_debug, verbosity, msbuild, use_scons, ms_help_compiler)
        if arch == "64":
            builder = Windows64bitBuilder(opticks_depends, arcsdk,
                build_in_debug, verbosity, msbuild, use_scons, ms_help_compiler)
    elif sys.platform.startswith("linux"):
        builder = LinuxBuilder(opticks_depends, arcsdk, build_in_debug, verbosity)
    else:
        builder = SolarisBuilder(opticks_depends, arcsdk,
            build_in_debug, verbosity)
    return builder

def prep_to_run(opticks_depends, build_debug, arch,
                msbuild, build_dir, verbosity):
    try:
        builder = create_builder(opticks_depends, False,
            build_debug, msbuild, False, None, arch, verbosity)
        if builder is not None:
            return builder.prep_to_run(build_dir)
    except ScriptException, se:
        return None


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
        options.add_option("--use-scons", action="store_true",
            dest="use_scons",
            help="If provided, compile using Scons "\
                 "on Windows instead of using vcbuild. Use "\
                 "if you don't have Visual C++ installed. "\
                 "The default is to use vcbuild")
        options.add_option("--arch", dest="arch", action="store",
            type="choice", choices=["32","64"], help="Use 32 or 64.")
        options.set_defaults(msbuild=msbuild_path,
            ms_help_compiler=ms_help_compiler_path, arch="64",
            use_scons=False)
    options.add_option("-m", "--mode", dest="mode",
        action="store", type="choice", choices=["debug", "release"],
        help="Use debug or release.")
    options.add_option("--clean", dest="clean", action="store_true")
    options.add_option("--build-opticks", dest="build_opticks",
        action="store", type="choice", choices=["all","core","none"],
        help="Use all, core or none.")
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
    options.add_option("--build-doxygen", dest="build_doxygen",
        action="store", type="choice", choices=["none", "html", "all"])
    options.add_option("--concurrency", dest="concurrency", action="store")
    options.add_option("-q", "--quiet", help="Print fewer messages",
        action="store_const",dest="verbosity", const=0)
    options.add_option("-v", "--verbose", help="Print more messages",
        action="store_const", dest="verbosity", const=2)
    options.set_defaults(mode="release",
       clean=False, build_opticks="none", update_version_scheme="none",
       prep=False, build_doxygen="none", concurrency=1, verbosity=1)

    options = options.parse_args(args[1:])[0]
    if not is_windows():
        options.msbuild = None
        options.ms_help_compiler = None
        options.arch = "64"
        options.use_scons = True 

    builder = None
    try:
        opticks_depends = os.environ.get("OPTICKSDEPENDENCIES", None)

        if options.dependencies:
            #allow the -d command-line option to override environment variable
            opticks_depends = options.dependencies
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
            options.use_scons,
            options.ms_help_compiler, options.arch, options.verbosity)
        if builder is None:
            raise ScriptException("Unable to create builder for platform")

        builder.update_app_version_number(options.update_version_scheme,
            options.new_version, options.release_date)

        if options.build_opticks != "none" or options.build_doxygen != "none":
            #Force BuildRevision.h to be up-to-date, after AppVersion.h
            #might have been changed but before we do anything else.
            builder.get_current_app_version()

        builder.build_executable(options.clean, options.build_opticks,
            options.concurrency)

        if options.build_doxygen != "none":
            if options.verbosity > 1:
                print "Building Doxygen..."
            builder.build_doxygen(options.build_doxygen, options.artifact_dir)
            if options.verbosity > 1:
                print "Done building Doxygen"

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
