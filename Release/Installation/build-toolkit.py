#!/usr/bin/env python
import os
from os.path import join
import os.path
import sys
from optparse import OptionParser
import traceback
import shutil
import subprocess
import zipfile

class ScriptException(Exception):
    def __init__(self, msg, retcode):
        Exception.__init__(self)
        self.msg = msg
        self.retcode = retcode

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
    shutil.copy2(source_file, join(destination_dir,
        os.path.basename(source_file)))

def cp_file3(source_file, destination_file):
    destination_dir = os.path.dirname(destination_file)
    if not(os.path.exists(destination_dir)):
        os.makedirs(destination_dir)
    shutil.copy2(source_file, destination_file)

def cp_dir2(source_base_dir, destination_base_dir, dir_to_copy,
            suffixes_to_match = [], exclude_matches = False,
            recursive_copy = False, dirs_to_exclude = [".svn", "_svn"]):
    full_src_dir = join(source_base_dir, dir_to_copy)
    full_dst_dir = join(destination_base_dir, dir_to_copy)
    return cp_dir(full_src_dir, full_dst_dir, suffixes_to_match,
        exclude_matches, recursive_copy, dirs_to_exclude)

def cp_dir(source_dir, destination_dir, suffixes_to_match = [],
           exclude_matches = False, recursive_copy = False,
           dirs_to_exclude = [".svn", "_svn"]):
    """Copy all files with or without specified suffix in source directory to
     destination directory

     @param source_dir: path of source directory
     @type source_dir: L{str}
     @param destination_dir: path of destination directory
     @type destination_dir: L{str}
     @param suffixes_to_match: a list of the specified suffixes to
                               match before files are copied.  This may
                               be an empty list and all files will match
                               and therefore be copied.
     @type suffixes_to_match: L{list} of L{str}
     @param exclude_matches: if False, only files that end with
                             suffix will be copied.  If True, only
                             files that don't end with suffix will be copied.
     @type exclude_matches: L{bool}
     @param recursive_copy: If True, the copy will be performed
                            recursively.  Note that the suffix to match
                            only applies to files and not directories.
     @type recursive_copy: L{bool}
     @param dirs_to_exclude: List of directories that should be
                             excluded when recursion is applied.
     @type dirs_to_exclude: L{list} of L{str}

     This function will create destination directory if it does not exist.
    """
    dir_contents = os.listdir(source_dir)
    symlinks = False
    if not(os.path.exists(destination_dir)):
        os.makedirs(destination_dir)
    for entry in dir_contents:
        src_path = join(source_dir, entry)
        dst_path = join(destination_dir, entry)
        if symlinks and os.path.islink(src_path):
            linkto = os.readlink(src_path)
            os.symlink(linkto, dst_path)
        elif os.path.isdir(src_path):
            if recursive_copy and dirs_to_exclude.count(entry) == 0:
                cp_dir(src_path, dst_path, suffixes_to_match,
                    exclude_matches, recursive_copy, dirs_to_exclude)
        else:
            matches = False
            if suffixes_to_match is None or len(suffixes_to_match) == 0:
                matches = True
            else:
                for suffix in suffixes_to_match:
                    if entry.endswith(suffix) != exclude_matches:
                        matches = True

            if matches:
                source_file = src_path
                if symlinks:
                    the_file = commonutils.isSubversionSoftLink(source_file)
                    if the_file is not None:
                        source_file = the_file

                shutil.copy2(source_file, dst_path)

def copy_windows_build(opticks_code_dir, sdk_dest_dir,
                       win_debug_dest_dir, static_libs, plugins,
                       is_32_bit, is_debug, verbosity):
    if is_32_bit:
        arch = "Win32"
    else:
        arch = "x64"
    if is_debug:
        mode = "debug"
    else:
        mode = "release"
    binaries_dir = join("Build", "Binaries-%s-%s" % (arch, mode))

    executables = ["Opticks.exe", "OpticksBatch.exe", "SimpleApiLib.dll"]
    for the_exec in executables:
        cp_file2(opticks_code_dir, sdk_dest_dir,
            join(binaries_dir, "Bin"), the_exec)

    for the_lib in static_libs:
        cp_file2(opticks_code_dir, sdk_dest_dir,
            join(binaries_dir, "Lib"), the_lib + ".lib")

    for the_plugin in plugins:
        cp_file2(opticks_code_dir, sdk_dest_dir,
            join(binaries_dir, "PlugIns"), the_plugin + ".dll")

    cp_dir2(opticks_code_dir, sdk_dest_dir,
        join(binaries_dir, "PlugIns", "ArcProxy"),
        suffixes_to_match = [".dll", ".exe"])

    if is_debug:
        #Copy the pdbs for the static_libs
        if verbosity > 1:
            print "Gathering pdb's for %s..." % (arch)
        all_pdbs = map(lambda x: os.path.splitext(x)[0], executables) + static_libs + plugins
        for the_file in all_pdbs:
            pdbs_dir = join(binaries_dir,"pdbs")
            cp_file2(opticks_code_dir, win_debug_dest_dir,
                pdbs_dir, the_file + ".pdb")
        if verbosity > 1:
            print "Done gathering pdb's for %s..." % (arch)

def copy_dir_into_zip(zip_file, parent_src_dir, the_dir,
                      prefix_dir, keep_the_dir=True):
    src_dir = join(parent_src_dir, the_dir)
    for root, dirs, files in os.walk(src_dir):
        try:
            dirs.remove(".svn")
        except Exception:
            pass
        try:
            dirs.remove("_svn")
        except Exception:
            pass
        if keep_the_dir:
            the_zip_dir = root[len(parent_src_dir)+1:]
        else:
            the_zip_dir = root[len(parent_src_dir) + len(the_dir) + 1:]
        for the_file in files:
            source_file = join(root, the_file)
            linked_file = commonutils.is_subversion_soft_link(source_file)
            if linked_file is not None:
                source_file = linked_file
            zip_file.write(source_file,
                join(prefix_dir, the_zip_dir, the_file))

def create_toolkit_zip(opticks_code_dir, opticks_dependencies_dir,
      package_dir, ms_help_compiler,
      release32, debug32, release64, debug64,
      verbosity):

    if opticks_dependencies_dir is None:
        if os.environ.has_key("OPTICKSDEPENDENCIES"):
            opticks_dependencies_dir = os.environ["OPTICKSDEPENDENCIES"]
        else:
            raise ScriptException("The path to the Opticks "\
                "dependencies was not provided, see -d")

    if not(os.path.exists(opticks_dependencies_dir)):
        raise ScriptException("The path to the Opticks dependencies "\
            "does not exist %s, see -d")

    if verbosity >= 1:
        print "Removing output from previous runs..."
    out_dir = os.path.abspath(join("Toolkit", "SDK-Temp"))
    win_debug_dir = os.path.abspath(join("Toolkit", "WinDebug-Temp"))
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir, False)

    if os.path.exists(win_debug_dir):
        shutil.rmtree(win_debug_dir, False)

    if verbosity >= 1:
        print "Done removing output from previous runs"

    if verbosity >= 1:
        print "Creating SDK..."
    app_version = commonutils.get_app_version_only(opticks_code_dir)
    ##### Create all the output directories
    os.makedirs(out_dir)
    os.makedirs(win_debug_dir)

    if verbosity > 1:
        print "Gathering files for SDK..."
    cp_file3("README-sdk.txt", join(out_dir, "README.txt"))
    s_app = os.path.abspath(join(opticks_code_dir, "application"))
    s_release = os.path.abspath(join(opticks_code_dir, "Release"))
    d_app = join(out_dir,"application")

    interface_suffixes = [".h"]
    cp_dir2(s_app, d_app, "Interfaces", suffixes_to_match=interface_suffixes)
    cp_dir2(s_app, d_app, join("PlugInUtilities", "Interfaces"),
        suffixes_to_match=interface_suffixes)
    cp_dir2(s_app, d_app, join("PlugInUtilities", "pthreads-wrapper"),
        suffixes_to_match=interface_suffixes)
    cp_dir2(s_app, d_app, "SimpleApiLib",
        suffixes_to_match=interface_suffixes)
    compile_settings_suffix = [".py"]
    if is_windows():
        compile_settings_suffix.append(".vsprops")
    cp_dir2(s_app, d_app, "CompileSettings",
        suffixes_to_match=compile_settings_suffix)
    cp_dir2(s_app, d_app, "PlugInLib", suffixes_to_match=interface_suffixes)
    cp_dir2(s_app, d_app, "HdfPlugInLib", suffixes_to_match=interface_suffixes)
    cp_dir2(s_app, d_app, "NitfPlugInLib",
        suffixes_to_match=interface_suffixes)

    #Copy the PlugInSamplerQt code to the right spot
    source_suffixes = interface_suffixes + [".cpp", ".ui"]
    if is_windows():
        source_suffixes.append(".vcproj")
    else:
        source_suffixes.append("SConscript")

    cp_dir2(s_app, d_app, join("PlugIns", "src", "PlugInSamplerQt"),
        suffixes_to_match=source_suffixes)
    cp_dir2(s_app, d_app, join("PlugIns", "src", "PlugInSampler"),
        suffixes_to_match=source_suffixes)
    cp_dir2(s_app, d_app, join("PlugIns", "src", "PlugInSamplerHdf"),
        suffixes_to_match=source_suffixes)
    cp_dir2(s_app, d_app, join("PlugIns", "src", "Tutorial"),
        suffixes_to_match=source_suffixes)
    cp_file2(s_app, d_app, join("PlugIns", "src", "Aspam"), "Aspam.h")

    if verbosity > 1:
        print "Done gathering files for SDK"

    win_debug_code_dir = join(win_debug_dir, "Code")
    if is_windows():
        if verbosity > 1:
            print "Exporting Opticks source code..."
        svn_export_code_args = list()
        svn_export_code_args.append("svn")
        svn_export_code_args.append("export")
        svn_export_code_args.append("-r")
        svn_export_code_args.append("BASE")
        svn_export_code_args.append(os.path.abspath(opticks_code_dir))
        svn_export_code_args.append(win_debug_code_dir)
        retcode = execute_process(svn_export_code_args)
        if retcode != 0:
            raise ScriptException("Unable to export code.")
        if verbosity > 1:
            print "Done exporting Opticks source code"

        cp_file3("README-pdb-source.txt",
            join(win_debug_dir, "README.txt"))

    #Copy dependencies
    if verbosity > 1:
        print "Exporting dependencies..."
    svn_export_args = list()
    svn_export_args.append("svn")
    svn_export_args.append("export")
    svn_export_args.append("-r")
    svn_export_args.append("BASE")
    svn_export_args.append(os.path.abspath(opticks_dependencies_dir))
    svn_export_args.append(join(out_dir, "Dependencies"))
    retcode = execute_process(svn_export_args)
    if retcode != 0:
        raise ScriptException("Unable to export dependencies")
    if verbosity > 1:
        print "Done exporting dependencies"

    ##### Run Doxygen to generate the html documentation
    if verbosity > 1:
        print "Generating Doxygen..."
    build_doxygen_args = ["python", join(os.path.abspath(opticks_code_dir),
        "build.py"),
        "--build-doxygen=all",
        "-d", opticks_dependencies_dir]
    if is_windows():
        build_doxygen_args.append("--ms-help-compiler=%s" % (ms_help_compiler))
    if verbosity == 2:
        build_doxygen_args.append("-v")
    if verbosity == 0:
        build_doxygen_args.append("-q")
    retcode = execute_process(build_doxygen_args)
    if retcode != 0:
        raise ScriptException("Error occurred while building "\
            "on-line help")
    s_doxygen_output = join(opticks_code_dir, "Build", "DoxygenOutput")
    d_doc_output = join(out_dir, "doc")
    cp_dir2(s_doxygen_output, d_doc_output, "html", recursive_copy = True)
    if is_windows():
        cp_file2(s_doxygen_output, d_doc_output, "",
            "OpticksSDK-%s.chm" % (app_version))
    if verbosity > 1:
        print "Done generating Doxygen"

    if verbosity > 1:
        print "Acquiring Opticks binaries..."
    static_libs = ["PlugInLib", "PlugInUtilities", "HdfPlugInLib", "NitfPlugInLib"]
    if is_windows():
       static_libs.append("SimpleApiLib")
    plugins = [ "AnnotationImagePalette", "Aspam", "AutoImporter", "BandMath", "ConvolutionFilter",
        "CoreIo", "Covariance", "DataFusion", "Dted", "ENVI", "Fits", "GdalImporter", "Generic",
        "GeographicFeatures", "Georeference", "Hdf", "Ice", "ImageComparison", "Kml",
        "MovieExporter", "Nitf", "NitfCommonTre", "ObjectFinding", "Pca", "Pictures", "Results",
        "Scripts", "SecondMoment", "ShapeFileExporter", "Sio", "Wavelength",
        "WizardExecutor", "WizardItems" ]
    sample_plugins = ["PlugInSampler", "PlugInSamplerQt",
        "PlugInSamplerHdf", "Tutorial" ]
    if is_windows():
        cp_file2(s_app, d_app, "", "SamplePlugIns.sln")
        cp_file2(s_app, d_app, "PlugInManager", "PlugInModule.def")

        plugins = plugins + sample_plugins
        #Win32 Build
        if debug32:
            copy_windows_build(opticks_code_dir, out_dir,
                win_debug_code_dir, static_libs,plugins, True, True, verbosity)
            dp_list = commonutils.get_dependencies(opticks_dependencies_dir,
                "Windows", True, "32")
            bin_dir = join(out_dir, "Build", "Binaries-win32-debug", "Bin")
            commonutils.copy_dependencies(dp_list, bin_dir)
        if release32:
            copy_windows_build(opticks_code_dir, out_dir,
                win_debug_code_dir, static_libs, plugins, True, False, verbosity)
            dp_list = commonutils.get_dependencies(opticks_dependencies_dir,
                "Windows", False, "32")
            bin_dir = join(out_dir, "Build", "Binaries-win32-release", "Bin")
            commonutils.copy_dependencies(dp_list, bin_dir)

        #Win64 Build
        if debug64:
            copy_windows_build(opticks_code_dir, out_dir,
                win_debug_code_dir, static_libs, plugins, False, True, verbosity)
            dp_list = commonutils.get_dependencies(opticks_dependencies_dir,
                "Windows", True, "64")
            bin_dir = join(out_dir, "Build", "Binaries-x64-debug", "Bin")
            commonutils.copy_dependencies(dp_list, bin_dir)
        if release64:
            copy_windows_build(opticks_code_dir, out_dir,
                win_debug_code_dir, static_libs, plugins, False, False, verbosity)
            dp_list = commonutils.get_dependencies(opticks_dependencies_dir,
                "Windows", False, "64")
            bin_dir = join(out_dir, "Build", "Binaries-x64-release", "Bin")
            commonutils.copy_dependencies(dp_list, bin_dir)
    else:
        cp_file2(s_app, d_app, join("PlugIns", "src"), "SConstruct")

        binaries_dir = join("Build", "Binaries-solaris-sparc-release")
        lib_dir = join(binaries_dir,"Lib")
        for the_lib in static_libs:
            cp_file2(opticks_code_dir, out_dir, lib_dir, "lib%s.a" % (the_lib))
        cp_file2(opticks_code_dir, out_dir, lib_dir, "libSimpleApiLib.so")
        cp_file2(opticks_code_dir, out_dir, lib_dir, "ModuleShell.os")

        for the_plugin in sample_plugins:
            cp_file2(opticks_code_dir, out_dir,
                join(binaries_dir,"PlugIns"), "%s.so" % (the_plugin))
        #Make copy of the "application" dir but with an upper-case
        #first letter to maintain compatibility with earlier SDKs
        cp_dir(d_app, join(out_dir, "Application"), recursive_copy = True)
    if verbosity > 1:
        print "Done acquiring Opticks binaries"
    if verbosity >= 1:
        print "Done building SDK"

    if package_dir is not None and os.path.exists(package_dir):
        if is_windows():
            zip_name = join(package_dir,
                "opticks-sdk-%s-windows.zip" % (app_version))
            if verbosity > 1:
                print "Creating compressed archive %s..." % (zip_name)
            zip_obj = zipfile.ZipFile(zip_name, "w", zipfile.ZIP_DEFLATED)
            copy_dir_into_zip(zip_obj, os.path.abspath("Toolkit"),
                "SDK-Temp", ".", False)
            zip_obj.close()
            if verbosity > 1:
                print "Done creating compressed archive"

            zip_name = join(package_dir,
                "opticks-pdb-sourcecode-%s-windows.zip" % (app_version))
            if verbosity > 1:
                print "Creating compressed archive %s..." % (zip_name)
            zip_obj = zipfile.ZipFile(zip_name, "w", zipfile.ZIP_DEFLATED)
            copy_dir_into_zip(zip_obj, os.path.abspath("Toolkit"),
                "WinDebug-Temp", ".", False)
            zip_obj.close()
            if verbosity > 1:
                print "Done creating compressed archive"
        else:
            output_tar_bz2 = os.path.abspath(join(package_dir,
                "opticks-sdk-%s-sol10-sparc.tar.bz2" % (app_version)))
            if verbosity > 1:
                print "Creating compressed archive %s..." % (output_tar_bz2)
            tar_args = list()
            tar_args.append("tar")
            tar_args.append("-cvf")
            tar_args.append("-")
            tar_args.append(".")
            tar = subprocess.Popen(tar_args, stdout=subprocess.PIPE,
                cwd=out_dir)

            output_handle = open(output_tar_bz2, "wb")
            bzip2_args = list()
            bzip2_args.append("bzip2")
            bzip2_args.append("-c")
            bzip2 = subprocess.Popen(bzip2_args, stdin=tar.stdout,
                stdout=output_handle)

            tar_ret = tar.wait()
            bzip_ret = bzip2.wait()
            output_handle.close()
            if tar_ret != 0:
                raise ScriptException("Running tar failed.")
            if bzip_ret != 0:
                raise ScriptException("Running bzip2 failed.")
            if verbosity > 1:
                print "Done creating compressed archive"

def is_windows():
    """Determine if this script is executing on the Windows operating system.
    @return: Return True if script is executed on Windows, False otherwise.
    @rtype: L{bool}

    """
    return sys.platform.startswith("win32")

def parse_args(args):
    ##### Parse the arguments
    if is_windows():
        desc = "Generate the Developers Toolkit .zip file for the "\
            "32-bit and 64-bit version of Opticks for Windows."
    else:
        desc = "Generate the Developers Toolkit file for the Solaris "\
            "version of Opticks."

    parser = OptionParser(usage="%prog [options]",
        version="%prog 1.0", description=desc)
    parser.add_option("-c", "--code-dir", action="store",
        dest="opticks_code_dir",
        help="The path to the checkout of the Code folder "\
        "from the Opticks trunk.")
    parser.add_option("-d", action="store", dest="dependencies",
        help="The path to Opticks dependencies")
    parser.add_option("--package-dir", action="store",
        dest="package_dir", help="The directory where the toolkit "\
        "output should be stored.  This directory must already exist.")
    if is_windows():
        ms_help_compiler_path = "C:\\Program Files (x86)\\HTML Help Workshop"
        if not os.path.exists(ms_help_compiler_path):
            ms_help_compiler_path = "C:\\Program Files\\HTML Help Workshop"
        parser.add_option("--ms-help-compiler",
            dest="ms_help_compiler", action="store", type="string")
        parser.add_option("--Nr32",
            help="Omit the release 32-bit binaries from the SDK",
            dest="release32", action="store_false")
        parser.add_option("--Nd32",
            help="Omit the debug 32-bit binaries from the SDK",
            dest="debug32", action="store_false")
        parser.add_option("--Nr64",
            help="Omit the release 64-bit binaries from the SDK",
            dest="release64", action="store_false")
        parser.add_option("--Nd64",
            help="Omit the debug 64-bit binaries from the SDK",
            dest="debug64", action="store_false")
        parser.set_defaults(ms_help_compiler=ms_help_compiler_path,
            release32=True, debug32=True,
            release64=True, debug64=True)
    parser.add_option("-q", "--quiet", help="Print fewer messages",
        action="store_const", dest="verbosity", const=0)
    parser.add_option("-v", "--verbose", help="Print more messages",
        action="store_const", dest="verbosity", const=2)
    parser.set_defaults(verbosity=1)

    #Parse the optional arguments, plus any additional arguments present
    #after optional arguments
    options = parser.parse_args(args[1:])[0]
    if not(is_windows()):
        options.ms_help_compiler = None
        options.release32 = None
        options.debug32 = None
        options.release64 = None
        options.debug64 = None

    return options

def main(args):
    options = parse_args(args)
    try:
        if options.opticks_code_dir is None:
            if os.environ.has_key("OPTICKS_CODE_DIR"):
                options.opticks_code_dir = os.environ["OPTICKS_CODE_DIR"]
            else:
                raise ScriptException("The path to the Opticks "\
                    "source code was not provided, see -c or --code-dir")

        if not(os.path.exists(options.opticks_code_dir)):
            raise ScriptException("The path to the Opticks source "\
                "code does not exist %s, see -c or --code-dir")

        sys.path.append(options.opticks_code_dir)
        import commonutils
        #the previous import line, only imports local to this function
        #so push import to global scope.
        globals()["commonutils"] = commonutils

        create_toolkit_zip(options.opticks_code_dir, options.dependencies,
           options.package_dir, options.ms_help_compiler,
           options.release32, options.debug32, options.release64,
           options.debug64, options.verbosity)
    except Exception, e:
        print "--------------------------"
        traceback.print_exc()
        print "--------------------------"
        return 2000
    return 0

#Main execution path when script is run
if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
    retcode = main(sys.argv)
    if retcode != 0:
        print "ERROR: Return code is %s" % (retcode)
    sys.exit(retcode)
