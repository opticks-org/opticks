#!/usr/local/bin/python
#
# The information in this file is
# Copyright(c) 2020 Ball Aerospace & Technologies Corporation
# and is subject to the terms and conditions of the
# GNU Lesser General Public License Version 2.1
# The license text is available from   
# http://www.gnu.org/licenses/lgpl.html

import sys
import subprocess
import os
import os.path
from os.path import join
import optparse
import traceback
import shutil
import datetime
import codecs
import re
import threading
import Queue

def execute_process(args, bufsize=0, executable=None, preexec_fn=None,
      close_fds=None, shell=False, cwd=None, env=None,
      universal_newlines=False, startupinfo=None, creationflags=0,
      output_fileobj=None):
    if is_windows():
        if output_fileobj is None:
            stdin = subprocess.PIPE
            stdout = sys.stdout
            stderr = sys.stderr
        else:
            stdin = subprocess.PIPE
            stdout = output_fileobj
            stderr = subprocess.STDOUT
    else:
        if output_fileobj is None:
            stdin = None
            stdout = None
            stderr = None
        else:
            stdin = None
            stdout = output_fileobj
            stderr = None

    process = subprocess.Popen(args, bufsize=bufsize, stdin=stdin,
          stdout=stdout, stderr=stderr, executable=executable,
          preexec_fn=preexec_fn, close_fds=close_fds, shell=shell,
          cwd=cwd, env=env, universal_newlines=universal_newlines,
          startupinfo=startupinfo, creationflags=creationflags)
    if is_windows():
        process.stdin.close()
    returncode = process.wait()
    return returncode

def is_windows():
    """Determine if this script is executing on the Windows operating system.
    @return: Return True if script is executed on Windows, False otherwise.
    @rtype: L{bool}

    """
    return sys.platform.startswith("win32")

class ScriptException(Exception):
    """Report error while running script"""

class TestbedWorker(threading.Thread):
    def __init__(self, queue, output_lock, all_lines, comment_out_lines,
                 cases_to_modify, verbosity, env, testbed_exe):
        threading.Thread.__init__(self)
        self.queue = queue
        self.output_lock = output_lock
        self.all_lines = all_lines
        self.comment_out_lines = comment_out_lines
        self.cases_to_modify = cases_to_modify
        self.verbosity = verbosity
        self.env = env
        self.combined_ret_code = 0
        self.testbed_exe = testbed_exe

    def run(self):
        try:
            while not(self.queue.empty()):
                (suite, num) = self.queue.get_nowait()
                with self.output_lock:
                    print "%s: Starting %s\n" % (self.getName(), suite)
                test_list_file = "TestList-%s.currentrun" % (self.getName())
                out_file = "TestRun-%s-Output.txt" % (self.getName())
                thout = open(out_file, "w")
                file_output = open(test_list_file, "w")
                count = 0
                for line in self.all_lines:
                    if count != num:
                        file_output.write("#")
                        file_output.write(line)
                    else:
                        file_output.write(line.strip())
                        try:
                            for case in self.cases_to_modify[suite]:
                                file_output.write(" " + case)
                        except Exception:
                            pass
                        file_output.write('\n')
                    count = count + 1
                file_output.close()
                arguments = []
                arguments.append(self.testbed_exe)
                if not is_windows():
                    prefix = "-"
                else:
                    prefix = "/"
                arguments.append("%scatchErrors:true" % (prefix))
                arguments.append("%stestList:%s" % (prefix, test_list_file))
                with self.output_lock:
                    print "%s: Starting Testbed executable for suite %s" % (self.getName(), suite)
                ret_code = execute_process(arguments,
                    output_fileobj=thout, env=self.env)
                with self.output_lock:
                    print "%s: Retcode from suite %s is %s" % (self.getName(), suite, ret_code)
                if ret_code != 0:
                    if self.verbosity >= 1:
                        thout.write("ERROR: %s did not complete "\
                            "successfully\n" % (suite))
                        thout.flush()
                    self.combined_ret_code = self.combined_ret_code + 1
                os.remove(test_list_file)

                thout.close()
                thout = open(out_file, "r")
                with self.output_lock: 
                    print "%s: Done with %s" % (self.getName(), suite)
                    shutil.copyfileobj(thout, sys.stdout)
                thout.close()
        except:
            self.combined_ret_code = self.combined_ret_code + 1000
            raise

class Builder:
    def __init__(self, dependencies, build_in_debug,
                 test_list, verbosity):
        self.depend_path = dependencies
        self.build_debug_mode = build_in_debug
        self.verbosity = verbosity
        if self.build_debug_mode:
            self.mode = "debug"
        else:
            self.mode = "release"
        self.test_list = test_list

    def build_testbed(self, clean_build_first, concurrency):
        #No return code, throw exception or ScriptException
        if self.verbosity > 1:
            print "Building TestBed..."
        buildenv = os.environ
        buildenv["OPTICKSDEPENDENCIES"] = self.depend_path

        if clean_build_first:
            if self.verbosity > 1:
                print "Cleaning compilation..."
            self.compile_testbed(buildenv, True, concurrency)
            if self.verbosity > 1:
                print "Done cleaning compilation"

        self.compile_testbed(buildenv, False, concurrency)
        if self.verbosity > 1:
            print "Done building TestBed"

    def update_version_number(self, scheme, new_version):
        if scheme is None or scheme == "none":
            return

        if self.verbosity > 1:
            print "Updating COMET version..."
        version_number = None
        branding_file = os.path.join("Release", "SupportFiles",
            "PlugInBranding", "comet.brand")
        if os.path.exists(branding_file):
            branding_handle = open(branding_file, "r")
            branding = branding_handle.read()
            branding_handle.close()
            version_match = re.search(r"<Version>(.*?)<\/Version>", branding)
            if version_match is not None:
                version_number = version_match.group(1)

        if version_number is None:
            raise ScriptException("Could not determine current "\
                "version while attempting to update the version")
        if self.verbosity >= 1:
            print "Original version # of Comet was", version_number

        if new_version is not None:
            version_number = new_version

        if scheme == "nightly" or scheme == "unofficial":
            #strip off any suffix from the version #
            version_parts = version_number.split(".")
            for count in range(0, len(version_parts) - 1):
                if not(version_parts[count].isdigit()):
                    raise ScriptException("The current app version # "\
                        "is improperly formatted.")
            last_part = version_parts[-1]
            match_obj = re.match("^\d+(\D*)", last_part)
            if match_obj is None:
                raise ScriptException("The current app version # is "\
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
                    raise ScriptException("This platform does not "\
                        "properly pad month and days to 2 digits when "\
                        "using strftime.  Please update this "\
                        "script to address this problem")
                version_number = version_number + "Nightly%s" % (today_str)
        elif new_version is None:
            if self.verbosity >= 1:
                print "You need to use --new-version to provide the "\
                    "version # when using the production, rc, or "\
                    "milestone scheme"

        if self.verbosity >= 1:
            print "Setting version # of COMET to", version_number

        #Update branding file
        branding_file = os.path.join("Release", "SupportFiles",
            "PlugInBranding", "comet.brand")
        if not(os.path.exists(branding_file)):
            return None
        branding_handle = open(branding_file, "r")
        branding = branding_handle.read()
        branding_handle.close()
        branding = re.sub(r"(<Version>).*?(<\/Version>)",
            r"\g<1>%s\2" % (version_number), branding)
        branding_handle = open(branding_file, "w")
        branding_handle.write(branding)
        branding_handle.close()
        if self.verbosity > 1:
            print "Done updating COMET version"

    def update_cfg_file(self, cfgfile, import_path):
        def importpath_repl(matchobj):
           return import_path

        # Update the import path setting in the .cfg file
        # to the test data location
        cfgfileHandle = codecs.open(cfgfile, "r", "utf-8")
        contents = cfgfileHandle.read()
        cfgfileHandle.close()
        contents = re.sub("TESTDATAPATH", importpath_repl, contents)
        cfgfileHandle = codecs.open(cfgfile, "w+", "utf-8")
        cfgfileHandle.write(contents)
        cfgfileHandle.close()

    def prep_to_run_helper(self, arch, msbuild_path, os_spec):
        if self.verbosity > 1:
            print "Prepping to run..."
        sys.path.append(os.path.abspath(os.environ["OPTICKS_CODE_DIR"]))
        import build
        import commonutils

        opticks_home = os.path.abspath(join(os.environ["OPTICKS_CODE_DIR"],
            "Release"))
        build_dir = os.path.abspath(join(os.environ["OPTICKS_CODE_DIR"],
            "Build"))
        defaults_dir = os.path.abspath(join(opticks_home,
            "TestBedDefaultSettings"))
        if not os.path.exists(defaults_dir):
            if self.verbosity > 1:
                print "Creating TestBedDefaultSettings "\
                    "folder at %s..." % (defaults_dir)
            os.makedirs(defaults_dir)
            if self.verbosity > 1:
                print "Done creating TestBedDefaultSettings folder"
        testbed_setting_dir = os.path.abspath(join(build_dir,
            "TestBedUserSettings"))
        if not os.path.exists(testbed_setting_dir):
            if self.verbosity > 1:
                print "Creating TestBedUserSettings "\
                    "folder at %s..." % (testbed_setting_dir)
            os.makedirs(testbed_setting_dir)
            if self.verbosity > 1:
                print "Done creating TestBedUserSettings folder"
        if self.verbosity > 1:
            print "Creating TestBed defaults config file..."
        script_dir = os.path.abspath(os.path.dirname(sys.argv[0]))
        shutil.copy2(join(script_dir, "TestBed-defaults.cfg"),
            join(defaults_dir, "50-TestBed-defaults.cfg"))
        self.update_cfg_file(join(defaults_dir, "50-TestBed-defaults.cfg"),
            self.import_path)
        if self.verbosity > 1:
            print "Done creating TestBed defaults config file..."
        if self.verbosity > 1:
            print "Prepping Opticks to run..."
        cur_dir = os.getcwd()
        os.chdir(os.path.abspath(os.environ["OPTICKS_CODE_DIR"]))
        ret_values = build.prep_to_run(self.depend_path,
            self.build_debug_mode, arch, msbuild_path,
            build_dir, self.verbosity)
        os.chdir(cur_dir)
        if self.verbosity > 1:
            print "Done prepping Opticks to run"

        bin_dir = join(self.get_binaries_dir(),"Bin")
        test_bin_dir = join(self.get_binaries_dir(),"TestBin")
        if self.verbosity > 1:
            print "Gathering dependency libraries to TestBin..."
        dp_list = commonutils.get_dependencies(self.depend_path,
            os_spec, self.build_debug_mode, arch)
        commonutils.copy_dependencies(dp_list, test_bin_dir)
        if self.verbosity > 1:
            print "Done gathering dependency libraries to TestBin"

        dep_file_path = os.path.join(test_bin_dir, "opticks.dep")
        if not(os.path.exists(dep_file_path)):
            if self.verbosity > 1:
                print "Creating opticks.dep file for TestBin..."
            dep_file = open(dep_file_path, "w")
            dep_file.write("!depV1 { deployment: { "\
                "AppHomePath: ../../../Release, "\
                "UserConfigPath: ../../TestBedUserSettings, "\
                "AdditionalDefaultPath: "\
                "../../../Release/TestBedDefaultSettings } }")
            dep_file.close()
            if self.verbosity > 1:
                print "Done creating opticks.dep file for TestBin"

        if os_spec == 'Windows':
           shutil.copy2(join(self.get_binaries_dir(), "Bin/SimpleApiLib.dll"), join(test_bin_dir, "SimpleApiLib.dll"))
        else:
           shutil.copy2(join(self.get_binaries_dir(), "Lib/libSimpleApiLib.so"), join(bin_dir, "libSimpleApiLib.so"))
        if self.verbosity > 1:
            print "Done prepping to run"
        return ret_values

    def run_app(self, executable, path, env=None, args=None,
                stdout_file = None):
        arguments = []
        arguments.append(executable)

        new_env = os.environ
        if env is not None:
            #Copy environment variables over
            for key in env.iterkeys():
                new_env[key] = env[key]

        if args is None:
            if not is_windows():
                prefix = "-"
            else:
                prefix = "/"
            arguments.append("%scatchErrors:true" % (prefix))
            arguments.append("%stestList:TestList.currentrun" % (prefix))
        else:
            arguments.extend(args)

        old_cwd_path = os.getcwd()
        os.chdir(path)
        if not is_windows():
            if stdout_file is not None:
                newstdout = open(stdout_file, "w")
            else:
                newstdout = os.dup(sys.stdout.fileno())
            proc = subprocess.Popen(arguments, stdin=subprocess.PIPE,
                stdout=newstdout, stderr=subprocess.STDOUT, env=new_env)
        else:
            proc = subprocess.Popen(arguments, stdin=subprocess.PIPE,
                stdout=sys.stdout)

        proc.stdin.close()
        ret_code = proc.wait()
        os.chdir(old_cwd_path)

        return ret_code

    def run_testbed_once_for_each_suite(self, testbed_exe, path, env,
                                        suites_to_skip,
                                        suites_to_always_run,
                                        cases_to_modify, concurrency):
        old_cwd = os.getcwd()
        os.chdir(path)

        #open original test list file
        test_list = open(self.test_list,"r")
        test_list_content = test_list.readlines()
        test_list.close()

        #Make suites_to_skip semantically correct with
        #suites_to_always_run
        for item in suites_to_always_run:
            try:
                suites_to_skip.remove(item)
            except Exception:
                pass

        queue = Queue.Queue()
        combined_ret_code = 0
        comment_out_lines = list()
        all_lines = list()
        count = 0
        #Find all the currently enabled test suite lines and
        #record their line #'s
        for line in test_list_content:
            run_it = False #assume we aren't going to run this TestSuite
            results = line.split(":")
            if len(results) == 2:
                suite = results[0].lstrip("#")
                #We have a valid TestSuite: line
                if line.startswith("#") or \
                    suites_to_skip.count("All") == 1:
                    if suites_to_always_run.count(suite) > 0:
                        #shouldn't have been run, but we found it
                        #in always run list
                        run_it = True
                elif suites_to_skip.count(suite) == 0:
                    #should be run and not in suites to skip, so run it
                    run_it = True
                if run_it:
                    queue.put( (suite, count) )

            count += 1
            all_lines.append(line)

        output_lock = threading.Lock()
        workers = []
        for i in range(concurrency):
            print "Creating worker %s" % (i)
            worker = TestbedWorker(queue, output_lock, all_lines,
                comment_out_lines, cases_to_modify, self.verbosity, env,
                testbed_exe)
            workers.append(worker)
            worker.setName("Worker%s" % (i))
            worker.start()

        for worker in workers:
            worker.join() #Wait for all threads to terminate
            combined_ret_code = combined_ret_code + worker.combined_ret_code

        os.chdir(old_cwd)

        return combined_ret_code

class WindowsBuilder(Builder):
    def __init__(self, dependencies, build_in_debug,
                 msbuild, import_path, test_list, verbosity):
        Builder.__init__(self, dependencies, build_in_debug,
            test_list, verbosity)
        self.msbuild_path = msbuild 
        if import_path:
            self.import_path = import_path
        else:
            self.import_path = "T:/cppTestData"

    def compile_testbed(self, env, clean, concurrency):
        solution_file = os.path.abspath("CppTests\\TestBed.sln")
        self.build_in_msbuild(solution_file,
            self.build_debug_mode, self.is_64_bit, concurrency,
            self.msbuild_path, env, clean)

    def get_binaries_dir(self):
        return join(os.environ["OPTICKS_CODE_DIR"], "Build",
            "Binaries-%s-%s" % (self.platform, self.mode))

    def run_testbed(self, suites_to_skip,
                    suites_to_always_run, cases_to_modify, concurrency):
        bin_dir = join(self.get_binaries_dir(),"TestBin")

        #Put support libraries where needed to run executable
        dll_path = self.prep_to_run()

        env = os.environ 
        env["PATH"] = dll_path

        #Run the actual testbed now
        ret_code =  self.run_testbed_once_for_each_suite(
            os.path.abspath(join(bin_dir, "TestBed.exe")), "CppTests", env,
            suites_to_skip, suites_to_always_run, cases_to_modify, concurrency)

        return ret_code

    def prep_to_run(self):
        if self.platform == "Win32":
            arch = "32"
        elif self.platform == "x64":
            arch = "64"
        return Builder.prep_to_run_helper(self, arch, self.msbuild_path, "Windows")

    def build_in_msbuild(self, solutionfile, debug,
                         build_64_bit, concurrency, msbuildpath,
                         environ, clean):
        if debug:
            config = "Debug"
        else:
            config = "Release"
        if build_64_bit:
            platform = "x64"
        else:
            platform = "Win32"

        msbuild_exec = os.path.join(msbuildpath, "msbuild.exe")
        arguments = [msbuild_exec, solutionfile]
        if clean:
            arguments.append("/target:clean")
        arguments.append("/m:%s" % concurrency)
        arguments.append("/p:Platform=%s" % platform)
        arguments.append("/p:Configuration=%s" % config)
        ret_code = execute_process(arguments,
                                 env=environ)
        if ret_code != 0:
            raise ScriptException("Visual Studio did not compile "\
                "project")

class Windows32bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, build_in_debug,
                 msbuild, import_path, test_list, verbosity):
        WindowsBuilder.__init__(self, dependencies, build_in_debug,
            msbuild, import_path, test_list, verbosity)
        self.is_64_bit = False
        self.platform = "Win32"

class Windows64bitBuilder(WindowsBuilder):
    def __init__(self, dependencies, build_in_debug,
                 msbuild, import_path, test_list, verbosity):
        WindowsBuilder.__init__(self, dependencies, build_in_debug,
            msbuild, import_path, test_list, verbosity)
        self.is_64_bit = True
        self.platform = "x64"

class SolarisBuilder(Builder):
    def __init__(self, dependencies, build_in_debug,
                 run_headless, import_path, test_list, verbosity):
        Builder.__init__(self, dependencies,  build_in_debug,
            test_list, verbosity)
        self.run_headless = run_headless
        if import_path:
            self.import_path = import_path
        else:
            self.import_path = "/TestData/cppTestData"

    def compile_testbed(self, env, clean, concurrency):
        #Build TestBed
        self.run_scons(os.path.abspath("CppTests"),
            self.build_debug_mode, concurrency, env, clean)

    def run_scons(self, path, debug, concurrency, environ,
                  clean, extra_args=None):
        scons_exec = "scons"
        arguments = [scons_exec, "--no-cache"]
        arguments.append("-j%s" % (concurrency))
        if clean:
            arguments.append("-c")
        if not debug:
            arguments.append("RELEASE=yes")
        if extra_args:
            arguments.extend(extra_args)
        ret_code = execute_process(arguments,
                                 env=environ,
                                 cwd=path)
        if ret_code != 0:
            raise ScriptException("Scons did not compile project")

    def get_binaries_dir(self):
        return join(os.environ["OPTICKS_CODE_DIR"],
            "Build","Binaries-solaris-sparc-%s" % (self.mode))

    def run_testbed(self, suites_to_skip, suites_to_always_run,
                    cases_to_modify, concurrency):
        bin_dir = join(self.get_binaries_dir(),"TestBin")

        lib_path = self.prep_to_run()

        env = os.environ 
        env["LD_LIBRARY_PATH_64"] = lib_path

        if self.run_headless:
            #start vncserver
            if self.verbosity > 1:
                print "Setting up VNCServer to run Testbed headless..."
            ret_code = self.run_app("vncserver", ".", None,
                ".", None, ["-pn", "-geometry", "1024x768", "-depth", "24"],
                "vncserveroutput.log")
            if ret_code != 0:
                return ret_code

            #parse output to determine display number
            displaynum = None
            vncserverfile = open("vncserveroutput.log", "r")
            for line in vncserverfile:
                if line.startswith("New 'X' desktop is"):
                    colonpos = line.find(":")
                    if colonpos != -1:
                        displaynum = int(line[colonpos+1:-1])
            vncserverfile.close()
            os.remove("vncserveroutput.log")
            if displaynum is None:
                raise ScriptException("Unable to retreive display num "\
                    "from vncserver output")
            print "--", displaynum, "--"
            env["DISPLAY"] = ":%s" % (str(displaynum))
            if self.verbosity > 1:
                print "Done setting up VNCServer"

        ret_code = self.run_testbed_once_for_each_suite(
            os.path.abspath(join(bin_dir,"testBed")),"CppTests",env,
            suites_to_skip, suites_to_always_run, cases_to_modify, concurrency)

        if self.run_headless:
            #kill vncserver
            if self.verbosity > 1:
                print "Tearing down VNCServer..."
            self.run_app("vncserver", ".", None, ".", None,
                ["-kill", ":%s" % (str(displaynum))], None)
            if self.verbosity > 1:
                print "Done tearing down VNCServer"

        return ret_code

    def prep_to_run(self):
        return Builder.prep_to_run_helper(self, "64", "", "Solaris")

def main(args):
    options = optparse.OptionParser()
    options.add_option("-d", "--dependencies", dest="dependencies",
        action="store", type="string")
    if is_windows():
        msbuild_path = "C:\\Windows\\Microsoft.NET\Framework\\v4.0.30319"
        options.add_option("--msbuild", dest="msbuild",
            action="store", type="string")
        options.add_option("--arch", dest="arch", action="store",
            type="choice", choices=["32","64"])
        options.set_defaults(msbuild=msbuild_path, arch="64")
    if not is_windows():
        options.add_option("--run-headless", dest="run_headless",
            action="store_true")
        options.set_defaults(run_headless=False)
    options.add_option("-m", "--mode", dest="mode",
        action="store", type="choice", choices=["debug", "release"])
    options.add_option("--clean", dest="clean", action="store_true")
    options.add_option("--build-testbed", dest="build_testbed",
        action="store_true")
    options.add_option("--run-testbed", dest="run_testbed",
        action="store_true")
    options.add_option("-r", "--run-suite", dest="suites_to_run",
        action="append")
    options.add_option("-s", "--skip-suite", dest="suites_to_skip",
        action="append")
    options.add_option("-c", "--case-modify", dest="cases_to_modify",
        action="append")
    options.add_option("--update-version", dest="update_version_scheme",
        action="store", type="choice",
        choices=["milestone", "nightly", "none", "production", "rc",
        "unofficial"],
        help="Use milestone, nightly, production, rc, unofficial or "\
        "none.  When using milestone, production, rc you will need "\
        "to use --new-version to provide the complete version. The "\
        "unofficial and nightly will mutate the existing version #, "\
        "so --new-version is not required.")
    options.add_option("--new-version", dest="new_version",
        action="store", type="string")
    options.add_option("--prep", dest="prep", action="store_true")
    options.add_option("-i", "--import-path", dest="import_path",
        action="store", type="string")
    options.add_option("--test-list", dest="test_list", action="store")
    options.add_option("--concurrency", dest="concurrency", action="store")
    options.add_option("-q", "--quiet", help="Print fewer messages",
        action="store_const", dest="verbosity", const=0)
    options.add_option("-v", "--verbose", help="Print more messages",
        action="store_const", dest="verbosity", const=2)
    options.set_defaults(mode="release", clean=False,
        build_testbed=False, run_testbed=False,
        update_version_scheme="none", prep=False,
        test_list="TestList.txt", concurrency=1, verbosity=1)
    options = options.parse_args(args[1:])[0]

    builder = None
    try:
        opticks_depends = os.environ.get("OPTICKSDEPENDENCIES", None)

        if options.dependencies:
            #allow the -d command-line option to override
            #environment variable
            opticks_depends = options.dependencies
        if not opticks_depends:
            #didn't use -d command-line option, nor an environment variable
            # so consider that an error
            raise ScriptException("Dependencies argument must "\
                "be provided")
        if not os.path.exists(opticks_depends):
            raise ScriptException("Dependencies path is invalid")

        if options.mode == "debug":
            build_in_debug = True
        else:
            build_in_debug = False

        if not is_windows():
            builder = SolarisBuilder(opticks_depends, build_in_debug,
                options.run_headless, options.import_path, options.test_list,
                options.verbosity)
        else:
            if not os.path.exists(options.msbuild):
                raise ScriptException("MSBuild path is invalid")

            if options.arch == "32":
                builder = Windows32bitBuilder(opticks_depends,
                    build_in_debug, options.msbuild, options.import_path,
                    options.test_list, options.verbosity)
            if options.arch == "64":
                builder = Windows64bitBuilder(opticks_depends,
                    build_in_debug, options.msbuild, options.import_path,
                    options.test_list, options.verbosity)

        builder.update_version_number(options.update_version_scheme,
            options.new_version)

        if options.prep or options.run_testbed or options.build_testbed:
            if not(os.environ.has_key("OPTICKS_CODE_DIR")):
                raise ScriptException("OPTICKS_CODE_DIR environment "\
                    "variable does not exist and is required.")

        if options.prep:
            ret_values = builder.prep_to_run()
            if ret_values is None:
                raise ScriptException("Prep to run did not succeed")

        if options.build_testbed:
            builder.build_testbed(options.clean, options.concurrency)

        if options.run_testbed:
            if options.verbosity > 1:
                print "Running TestBed..."
            cases_to_modify = dict()
            opt_cases_to_modify = options.cases_to_modify
            if opt_cases_to_modify is None:
                opt_cases_to_modify = list()

            for suite_and_case in opt_cases_to_modify:
                suite, cases = suite_and_case.split(":")
                cases_to_modify.setdefault(suite, list())
                cases_to_modify[suite] += cases.split(",")

            suites_to_skip = options.suites_to_skip
            suites_to_run = options.suites_to_run
            if suites_to_skip is None:
                suites_to_skip = list()
            if suites_to_run is None:
                suites_to_run = list()
            ret_code = builder.run_testbed(suites_to_skip, suites_to_run,
                cases_to_modify, int(options.concurrency))
            if options.verbosity > 1:
                print "Done running TestBed"
            if ret_code != 0:
                return ret_code

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
