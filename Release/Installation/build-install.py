#!/usr/local/bin/python

"""Generate installation packages for Opticks on both Windows and Solaris.

On Windows, this script uses the Windows Installer XML Toolset
(i.e. wix.sourceforge.net) to generate a .msi for the Windows 32-bit
version and a .msi for the Windows 64-bit version of Opticks.

On Solaris, this script uses Solaris Package commands (i.e. man pkgmk)
to generate a Solaris package for Opticks.

"""
import subprocess
import os
import os.path
import sys
import optparse
import traceback
import re
import zipfile
import shutil
import uuid

class ScriptException(Exception):
    """Report error while running script"""

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

def run_wix(input_files, wix_path, variables, extensions, culture,
            suppress_warnings, output_path, output_name = None):
    """Run Wix to generate a .msi from a .wxs file.

    Runs candle.exe and light.exe from the Wix Tool to generate a
    .msi file from a .wxs file.
    @param input_files: Path to the .wxs (i.e. Wix xml file) to be built
                        into the .msi
    @type input_files: L{list} of L{str}
    @param wix_path: Path to where the Wix Installation is located,
                     i.e. where candle.exe and light.exe are located
    @type wix_path: L{str}
    @param variables: The key/value pairs that will be passed as
                      candle.exe and light.exe as variables when
                      creating the installation.
    @type variables: L{dict} with values of type L{str}
    @param extensions: A list of Wix extensions that should be
                       compiled into the .msi.  None can be provided.
    @type extensions: L{list} of type L{str}
    @param culture: The name of the culture to use for any Wix extensions.
                    None can be provided.
    @type culture: L{str}
    @param suppress_warnings: The list of warning numbers to suppress
                              when running light.exe, i.e. -sw<N>
    @type suppress_warnings: L{list} of type L{str}
    @param output_path: The directory where the resulting .wixobj and
                        .msi should be generated to.
    @type output_path: L{str}
    @param output_name: The name of the resulting .msi and .wixobj file
                        generated to the output_path directory.  The
                        default is None, which means the name of the
                        first input_file .wxs file will be used.
    @type output_name: L{str}
    @return: Return zero on success, non-zero on failure.
    @rtype: L{int}

    """
    if len(input_files) == 0:
        return 1
    candle_args = list()
    candle_args.append(os.path.join(wix_path,"candle.exe"))
    for key in variables.iterkeys():
        candle_args.append("-d%s=%s" % (key, variables[key]))
    if output_name is None:
        output_name = os.path.splitext(os.path.basename(input_files[0]))[0]
    abs_output_path = os.path.abspath(output_path)
    if not(os.path.exists(abs_output_path)):
        os.makedirs(abs_output_path)
    candle_args.append("-out")
    #the join with empty string is to add trailing slash,
    #which candle.exe requires
    candle_args.append(os.path.join(abs_output_path, ""))
    candle_args.append("-wx")
    if extensions is not None:
        for ext in extensions:
            candle_args.append("-ext")
            candle_args.append(ext)
    for the_file in input_files:
        candle_args.append(the_file)
    print "Compiling...", " ".join(candle_args)
    retcode = execute_process(candle_args)
    if retcode != 0:
        return retcode

    light_args = list()
    light_args.append(os.path.join(wix_path,"light.exe"))
    for key in variables.iterkeys():
        light_args.append("-d%s=%s" % (key, variables[key]))
    light_args.append("-out")
    msi_file = os.path.join(abs_output_path, output_name) + ".msi"
    light_args.append(msi_file)
    light_args.append("-wx")
    for warning in suppress_warnings:
        light_args.append("-sw%s" %(warning))
    if extensions is not None:
        for ext in extensions:
            light_args.append("-ext")
            light_args.append(ext)
    if culture is not None:
        light_args.append("-cultures:%s" % (culture))

    for the_file in input_files:
        wix_obj_file = os.path.join(abs_output_path,
            os.path.splitext(os.path.basename(the_file))[0] + ".wixobj")
        light_args.append(wix_obj_file)
    print "Linking...", " ".join(light_args)
    retcode = execute_process(light_args)
    return retcode

class WixDirectoryGenerator:
    def __init__(self, root_directory, path_to_search, disk_id,
                 space_per_level):
        self.root_directory = root_directory
        self.path_to_search = path_to_search
        self.disk_id = disk_id
        self.space_per_level = space_per_level

    def generate_helper(self, file_object, initial_level):
        self.walk_dir(self.path_to_search, initial_level, file_object)

    def walk_dir(self, path, level, output):
        total_path = os.path.join(self.root_directory, path)
        directories = list()
        files = list()
        for filename in os.listdir(total_path):
            total_current_path = os.path.join(total_path, filename)
            if os.path.isdir(total_current_path):
                directories.append(filename)
            else:
                files.append(filename)

        padding = " " * self.space_per_level * level
        output.write('%s<Directory Id="%s" Name="%s" '\
            'DiskId="%s" FileSource="%s">\n' % (padding,
            self.directory_id(path), self.directory_name(path),
            self.disk_id, self.directory_file_source(path)))

        if len(files) != 0:
            padding = " " * self.space_per_level * (level + 1)
            output.write('%s<Component Id="%s" Guid="%s" Win64="%s">\n' % \
                (padding, self.component_id(path),
                self.component_guid(path), self.component_64bit(path)))

        for filename in files:
            padding = " " * self.space_per_level * (level + 2)
            output.write('%s<File Name="%s" Id="%s" />\n' % (padding,
                self.file_name(path, filename), self.file_id(path, filename)))

        if len(files) != 0:
            padding = " " * self.space_per_level * (level + 1)
            output.write("%s</Component>\n" % (padding))

        for the_dir in directories:
            relative_current_dir = os.path.join(path, the_dir)
            self.walk_dir(relative_current_dir, level+1, output)

        padding = " " * self.space_per_level * level
        output.write("%s</Directory>\n" % (padding))

class HelpGenerator(WixDirectoryGenerator):
    def __init__(self, root_directory, path_to_search, disk_id,
                 wix_output_file, extra_output):
        WixDirectoryGenerator.__init__(self, root_directory,
            path_to_search, disk_id, 3)
        self.file_dict = dict()
        self.wix_output_file = wix_output_file
        self.extra_output = extra_output
    def generate(self):
        self.path_identifier = dict()
        self.component_ids = dict()
        self.component_guids = dict()
        output = open(self.wix_output_file, "w")
        output.write("""<?xml version="1.0" encoding="utf-8"?>
<?include Opticks-Version.wxi ?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi"
     RequiredVersion="3.0.2925.0">
   <Fragment Id="Opticks-Help">
      <DirectoryRef Id="TARGETDIR">\n""")
        WixDirectoryGenerator.generate_helper(self, output, 3)
        output.write("""      </DirectoryRef>
   </Fragment>
</Wix>""")
        output.close()
        self.extra_output.write("Component IDs\n")
        for key in self.component_ids.keys():
            self.extra_output.write('<ComponentRef Id="%s" />' % (key))
            self.extra_output.write("\n")
        self.extra_output.write("\nComponent GUIDs\n")
        for guid in self.component_guids.keys():
            self.extra_output.write('<?define %s = "" ?>' % (guid))
            self.extra_output.write("\n")

        return 0
    def get_path_identifier(self, path):
        last_path_element = os.path.basename(path)
        last_path_element = last_path_element.replace("-", "_")
        last_path_element = last_path_element.replace(" ", "_")
        last_path_element = last_path_element.replace("&", "")
        last_path_element = last_path_element.lower()

        if self.path_identifier.has_key(last_path_element):
            path_list = self.path_identifier[last_path_element]
            try:
                index = path_list.index(path)
                if index == 0:
                    return last_path_element
                else:
                    return "%s%s" % (last_path_element, index)
            except Exception:
                path_list.append(path)
                return "%s%s" % (last_path_element, len(path_list))
        else:
            self.path_identifier[last_path_element] = [path]
            return last_path_element
    def directory_id(self, path):
        return "D__Help_%s" % (self.get_path_identifier(path))
    def directory_name(self, path):
        if path == self.path_to_search:
            return self.path_to_search
        else:
            return os.path.basename(path)
    def directory_file_source(self, path):
        trimmed_path = path[len(self.path_to_search):]
        return "$(var.OpticksHelpDir)%s" % (trimmed_path)
    def component_id(self, path):
        key = "C__Help_%s" % (self.get_path_identifier(path))
        self.component_ids[key] = 1
        return key
    def component_guid(self, path):
        guid = "C__Help_%s_Guid" % (self.get_path_identifier(path))
        self.component_guids[guid] = 1
        return "$(var.%s)" % (guid)
    def component_64bit(self, path):
        return "no"
    def file_name(self, path, filename):
        clean_filename = filename.replace("&", "&amp;")
        return clean_filename
    def file_id(self, path, filename):
        return "F__Help_%s" % \
            (self.get_path_identifier(os.path.join(path, filename)))

class CommonBuilder:
    def __init__(self, opticks_code_dir, verbosity):
        self.opticks_code_dir = opticks_code_dir
        self.verbosity = verbosity

    def require_code_dir(self):
        if self.opticks_code_dir is None:
            if os.environ.has_key("OPTICKS_CODE_DIR"):
                self.opticks_code_dir = os.environ["OPTICKS_CODE_DIR"]
            else:
                raise ScriptException("The path to the "\
                    "Opticks source code was not provided, see "\
                    "-c or --code-dir")

        if not(os.path.exists(self.opticks_code_dir)):
            raise ScriptException("The path to the Opticks "\
                "source code does not exist %s, see -c or "\
                "--code-dir" % (self.opticks_code_dir))


class WixBuilder(CommonBuilder):
    def __init__(self, wix_path, opticks_code_dir, opticks_dependencies_dir,
                 opticks_release_dir, package_dir, verbosity):
        """Construct a Wix Builder object
        @param wix_path: See L{run_wix}.
        @type wix_path: L{str}
        @param opticks_code_dir: The path to the Code folder
                                 checkout of the Opticks trunk, with
                                 a release 32-bit and release 64-bit
                                 build performed.
        @type opticks_code_dir: L{str}
        @param opticks_dependencies_dir: The path the Dependencies
                                         checkout of the OpticksSupport trunk.
        @type opticks_dependencies_dir: L{str}
        @param opticks_release_dir: The path to the Release folder
                                    checkout of the Opticks trunk.
        @type opticks_release_dir: L{str}
        @param package_dir: The path where the installer packages
                            should be placed.
        @type package_dir: L{str}
        @param verbosity: The chatiness level when outputting messages.
                          0 - quiet, 1 - normal, 2 - verbose
        @type verbosity: L{int}

        """
        CommonBuilder.__init__(self, opticks_code_dir, verbosity)
        self.wix_path = wix_path
        self.opticks_dependencies_dir = opticks_dependencies_dir
        self.opticks_release_dir = opticks_release_dir
        self.output_dir = os.path.abspath("WixOutput")
        self.package_dir = package_dir

    def generate_installer(self, is_64_bit):
        """Generate either the Opticks-32bit.msi to the Wix32Output
        folder or Opticks-64bit.msi to the Wix64Output folder.
        @param is_64_bit: True if the 64-bit version of the
                          .msi should be built, False if the
                          32-bit version should be built.
        @type is_64_bit: L{bool}
        @rtype: L{int}

        """
        if self.verbosity > 1:
            print "Generating MSI..."

        if self.wix_path is None:
            raise ScriptException("The path to the Windows "\
                "Installer XML Toolset was not provided, see --wix-path")

        if not(os.path.exists(self.wix_path)):
            raise ScriptException("The path to Windows Installer "\
                "XML Toolset, %s is invalid" % (self.wix_path))

        self.require_code_dir()

        if self.opticks_dependencies_dir is None:
            if os.environ.has_key("OPTICKSDEPENDENCIES"):
                self.opticks_dependencies_dir = \
                    os.environ["OPTICKSDEPENDENCIES"]
            else:
                raise ScriptException("The path to the Opticks "\
                    "dependencies was not provided, see -d or "\
                    "--dependencies")

        if not(os.path.exists(self.opticks_dependencies_dir)):
            raise ScriptException("The path to the Opticks "\
                "dependencies does not exist %s, see -d or "\
                "--dependencies" % (self.opticks_dependencies_dir))

        if self.opticks_release_dir is None:
            raise ScriptException("The path to the Opticks release "\
                "directory was not provided, see -r or --release-dir")

        if not(os.path.exists(self.opticks_release_dir)):
            raise ScriptException("The path to the Opticks release "\
                "directory does not exist %s, see -r or "\
                "--release-dir" % (self.opticks_release_dir))

        #determine version # stored in Opticks-Version.wxi
        opticks_version_file = open("Opticks-Version.wxi", "r")
        opticks_version_contents = opticks_version_file.read()
        opticks_version_file.close()

        match_obj = re.search(r'PublicVersionNumber\s*?\=\s*?"(.*?)"',
            opticks_version_contents)
        version_number = "Unknown"
        if match_obj:
            version_number = match_obj.group(1)

        ivars = dict()
        ivars["OpticksCodeDir"] = self.opticks_code_dir
        if is_64_bit:
            msi_name = "Opticks-%s-64bit" % (version_number)
            ivars["Is64Bit"] = "True"
            ivars["OpticksBinariesDir"] = \
                os.path.join(self.opticks_code_dir, "Build",
                "Binaries-x64-Release")
            output_path = os.path.join(self.output_dir, "64")
            ivars["OpticksDependenciesDir"] = \
                self.get_dependencies(True, output_path)
            ivars["Opticks32BitDependenciesDir"] = \
                self.get_dependencies(False,
                os.path.join(self.output_dir, "32"))
            ivars["VisualStudioRuntimeDir"] = \
                os.path.abspath(os.path.join("VisualStudioRuntime", "x64"))
        else:
            msi_name = "Opticks-%s-32bit" % (version_number)
            ivars["Is32Bit"] = "True"
            ivars["OpticksBinariesDir"] = \
                os.path.join(self.opticks_code_dir, "Build",
                "Binaries-Win32-Release")
            output_path = os.path.join(self.output_dir, "32")
            ivars["OpticksDependenciesDir"] = \
                self.get_dependencies(False, output_path)
            ivars["Opticks32BitDependenciesDir"] = \
                ivars["OpticksDependenciesDir"]
            ivars["VisualStudioRuntimeDir"] = \
                os.path.abspath(os.path.join("VisualStudioRuntime", "Win32"))

        ivars["OpticksHelpDir"] = self.get_help()
        ivars["VisualStudio32BitRuntimeDir"] = \
            os.path.abspath(os.path.join("VisualStudioRuntime", "Win32"))
        ivars["OpticksReleaseDir"] = \
            os.path.abspath(self.opticks_release_dir)

        if self.verbosity > 1:
            print "Generating other ClassificationLevel files by "\
            "using ClassificationLevels-T.txt as the base..."
        self.generate_classification_file(os.path.join(self.output_dir,
            "ClassificationLevels-U.txt"), 1)
        self.generate_classification_file(os.path.join(self.output_dir,
            "ClassificationLevels-C.txt"), 2)
        self.generate_classification_file(os.path.join(self.output_dir,
            "ClassificationLevels-S.txt"), 3)
        ivars["OpticksClassificationDir"] = self.output_dir
        if self.verbosity > 1:
            print "Done generating other ClassificationLevel files"

        installer_created = run_wix(["Opticks.wxs", "Opticks-Help.wxs",
            "Opticks-UI.wxs"],
            self.wix_path,
            ivars,
            ["WixUtilExtension"],
            "en-us",
            ["1076"],
            output_path,
            msi_name)

        if installer_created != 0:
            raise ScriptException("Running Wix failed to generate "\
                "the installer, the Wix error code "\
                "is %s" % (installer_created))

        if self.package_dir is not None and os.path.exists(self.package_dir):
            if is_64_bit:
                platform = "64Bit"
            else:
                platform = "32Bit"
            zip_name = os.path.abspath(os.path.join(self.package_dir,
                "opticks-%s-windows-%s.zip" % (version_number, platform)))
            if self.verbosity > 1:
                print "Creating compressed archive %s..." % (zip_name)
            zip_obj = zipfile.ZipFile(zip_name, "w", zipfile.ZIP_DEFLATED)
            zip_obj.write(os.path.abspath(os.path.join(output_path,
                msi_name + ".msi")), msi_name + ".msi")
            zip_obj.write(os.path.abspath(os.path.join("DownloadDocs",
                "%sMsi" % (platform), "INSTALLING.txt")), "INSTALLING.txt")
            zip_obj.close()
            if self.verbosity > 1:
                print "Done creating compressed archive"
        if self.verbosity > 1:
            "Done generating MSI"

    def generate_classification_file(self, output_file, linecount):
        class_file = open("ClassificationLevels-T.txt", "r")
        output = open(output_file, "w")
        count = 4
        for line in class_file:
            if count <= linecount:
                output.write(line)
            count = count - 1
        output.close()

    def update_version(self, display_version):
        """Updates the version number that is used when generating the .msi.

        Updates the Opticks-Version.wxi file to have the new version number
        and also updates the product and upgrades codes for the 32-bit and
        64-bit version of the .msi package.

        @param display_version: The version in the format of major.minor.
        The version # provided is used for display purposes.  The internal
        MSI version # will always be 1.0.0 since the GUIDs will be updated.
        @type display_version: L{str}

        @rtype: L{int}

        """
        if self.verbosity > 1:
            print "Updating MSI version #..."
        msi_version = "1.0.0"
        if display_version == "current":
            self.require_code_dir()
            sys.path.append(self.opticks_code_dir)
            import commonutils
            display_version = \
                commonutils.get_app_version_only(self.opticks_code_dir)
            if self.verbosity >= 1:
                print "Detected application version as %s" % (display_version)

        if self.verbosity > 1:
            print "Setting MSI Display Version # to %s" % (display_version)
        if self.verbosity > 1:
            print "Setting MSI Internal Version # to %s" % (msi_version)
        version_file = open("Opticks-Version.wxi", "r")
        version_file_contents = version_file.read()
        version_file.close()

        version_file_contents = \
            re.sub(r'(PublicVersionNumber\s*?\=\s*?").*?(")',
            r"\g<1>%s\g<2>" % (display_version), version_file_contents)
        version_file_contents = \
            re.sub(r'(InternalVersionNumber\s*?\=\s*?").*?(")',
            r"\g<1>%s\g<2>" % (msi_version), version_file_contents)
        guid_matcher = \
            re.compile('<\?\s*?define\s*?\S+?Guid\s*?\=\s*?"(?P<guid>.*?)"')
        current_pos = 0
        if self.verbosity > 1:
            print "Updating GUID's..."
        while True:
            match_obj = guid_matcher.search(version_file_contents, current_pos)
            if match_obj:
                new_uuid = str(uuid.uuid4())
                version_file_contents = \
                    version_file_contents[:match_obj.start(1)] + \
                    new_uuid + version_file_contents[match_obj.end(1):]
                current_pos = match_obj.start(1)
            else:
                break
        if self.verbosity > 1:
            print "Done updating GUIDs"

        version_file = open("Opticks-Version.wxi", "w")
        version_file.write(version_file_contents)
        version_file.close()

        if self.verbosity >= 1:
            print "Opticks-Version.wxi has been updated to reference "\
                "the new version. You must re-run this script with "\
                "--32 or --64 to generate a .msi that uses the "\
                "new version number.  Please commit the "\
                "Opticks-Version.wxi change to Subversion."
        if self.verbosity > 1:
            print "Done updating MSI version #"

    def get_dependencies(self, is_64_bit, output_path):
        if is_64_bit:
            platform = "64"
        else:
            platform = "32"
        dependencies_output = os.path.join(output_path, "Dependencies")
        if not(os.path.exists(dependencies_output)):
            if self.verbosity > 1:
                print "Gathering dependency libraries for %s "\
                "platform..." % (platform)
            sys.path.append(self.opticks_code_dir)
            import commonutils

            dependencies_list = \
                commonutils.get_dependencies(self.opticks_dependencies_dir,
                "Windows", False, platform)
            commonutils.copy_dependencies(dependencies_list,
                dependencies_output)
            if self.verbosity > 1:
                print "Done gathering dependency libraries"
        return dependencies_output

    def generate_wix_for_help(self):
        if self.verbosity > 1:
            print "Generating Opticks-Help.wxs from checked-in help..."
        self.get_help()
        help_output = os.path.join(self.output_dir, "Help")
        if self.verbosity > 1:
            print "Creating Opticks-Help.wxs by traversing "\
            "contents of Help dir at %s..." % (help_output)
        generator = HelpGenerator(self.output_dir, "Help", "1",
            "Opticks-Help.wxs", sys.stdout)
        retcode = generator.generate()
        if retcode != 0:
            raise ScriptException("There was an error while generating "\
                "the Opticks-Help.wxs")
        if self.verbosity >= 1:
            print "The Opticks-Help.wxs has been replaced by recursively "\
            "walking the directory structure of the unzipped Help.  "\
            "Above you will see the Component IDs that you be "\
            "inserted into Opticks.wxs.  You will also see the "\
            "Component GUIDs that should be inserted into "\
            "Opticks-Version.wxi and then this script should be "\
            "re-run with the --update-version argument."
        if self.verbosity > 1:
            print "Done generating Opticks-Help.wxs"


    def get_help(self):
        help_output = os.path.join(self.output_dir, "Help")
        if not(os.path.exists(help_output)):
            help_zip_path = os.path.join(self.opticks_release_dir,
                "Help", "Opticks.zip")
            total_help_output = os.path.join(help_output, "Opticks")
            if self.verbosity > 1:
                print "Unpacking Help located at %s "\
                 "to %s..." % (help_zip_path, total_help_output)
            unzip_file(help_zip_path, total_help_output)
            if self.verbosity > 1:
                print "Done unpacking Help"
        return help_output

    def clean(self):
        if os.path.exists(self.output_dir):
            shutil.rmtree(self.output_dir)


def run_pkgmk(variables, output_path):
    """Runs pkgmk with the given variables to generate a package in
    the given output_path.

    This function will run pkgmk (i.e. man pkgmk) to generate a Solaris package.  It will
    then run pkgchk (i.e. man pkgchk) to check the generated Solaris package to make sure
    it is correct.
    @param variables: The key/value pairs that will be passed to pkgmk when creating the package.
    @type variables: L{dict}
    @param output_path: The directory that the package should be generated to.
    @type output_path: L{string}
    @return: Returns zero on success, non-zero on failure.
    @rtype: L{int}

    """
    pkgmk_args = list()
    pkgmk_args.append("pkgmk")
    pkgmk_args.append("-o")
    pkgmk_args.append("-d")
    pkgmk_args.append(output_path)
    for key in variables.iterkeys():
        pkgmk_args.append("%s=%s" % (key, variables[key]))

    if not(os.path.exists(output_path)):
        os.makedirs(output_path)
    print "Building Package", " ".join(pkgmk_args)
    ret_code = execute_process(pkgmk_args)
    if ret_code != 0:
        return ret_code
    return execute_process(("pkgchk -d %s" % (output_path)).split(" "))

class PackageBuilder(CommonBuilder):
    def __init__(self, opticks_code_dir, opticks_dependencies_dir,
                 opticks_release_dir, package_dir, verbosity):
        CommonBuilder.__init__(self, opticks_code_dir, verbosity)
        self.output_dir = os.path.abspath("PackageOutput")
        self.opticks_dependencies_dir = opticks_dependencies_dir
        self.opticks_release_dir = opticks_release_dir
        self.package_dir = package_dir

    def clean(self):
        shutil.rmtree(self.output_dir, True)

    def update_version(self, new_version):
        if self.verbosity > 1:
            print "Updating pkg version #..."
        if new_version == "current":
            self.require_code_dir()
            sys.path.append(self.opticks_code_dir)
            import commonutils
            new_version = \
                commonutils.get_app_version_only(self.opticks_code_dir)
            if self.verbosity >= 1:
                print "Detected application version as %s" % (new_version)

        pkginfo_file = open("pkginfo", "r")
        pkginfo = pkginfo_file.read()
        pkginfo_file.close()
        pkginfo = re.sub(r"VERSION=.+?\n", "VERSION=%s\n" % (new_version),
            pkginfo)
        pkginfo_file = open("pkginfo", "w")
        pkginfo_file.write(pkginfo)
        pkginfo_file.close()
        if self.verbosity >= 1:
            print "pkginfo has been updated to reference the new version. "\
            "You must re-run this script to generate a Solaris Package "\
            "that uses the new version number.  Please commit the pkginfo "\
            "change to Subversion."
        if self.verbosity > 1:
            print "Done updating pkg version #"

    def get_data_from_pkginfo(self, param):
        pkginfo_file = open("pkginfo", "r")
        pkginfo = pkginfo_file.read()
        pkginfo_file.close()
        parammatch = re.search(param + r"=(.+?)\n", pkginfo)
        if parammatch is not None:
            value = parammatch.group(1)
            return value
        return ""

    def generate_installer(self):
        """Generate the Opticks Solaris package to the PackageOutput folder.
        @rtype: L{int}

        """
        if self.verbosity > 1:
            print "Generating Solaris package..."
        if self.opticks_code_dir is None:
            if os.environ.has_key("OPTICKS_CODE_DIR"):
                self.opticks_code_dir = os.environ["OPTICKS_CODE_DIR"]
            else:
                raise ScriptException("The path to the Opticks source "\
                    "code was not provided, see -c or --code-dir")

        if not(os.path.exists(self.opticks_code_dir)):
            raise ScriptException("The path to the Opticks source "\
                "code does not exist %s, see -c or "\
                "--code-dir" % (self.opticks_code_dir))

        if self.opticks_dependencies_dir is None:
            if os.environ.has_key("OPTICKSDEPENDENCIES"):
                self.opticks_dependencies_dir = \
                    os.environ["OPTICKSDEPENDENCIES"]
            else:
                raise ScriptException("The path to the Opticks "\
                    "dependencies was not provided, see -d or "\
                    "--dependencies")

        if not(os.path.exists(self.opticks_dependencies_dir)):
            raise ScriptException("The path to the Opticks "\
                "dependencies does not exist %s, see -d or "\
                "--dependencies" % (self.opticks_dependencies_dir))

        if self.opticks_release_dir is None:
            raise ScriptException("The path to the Opticks release"
                "directory was not provided, see -r or --release-dir")

        if not(os.path.exists(self.opticks_release_dir)):
            raise ScriptException("The path to the Opticks release"\
                "directory does not exist %s, see -r or "\
                "--release-dir" % (self.opticks_release_dir))

        ivars = dict()
        ivars["OpticksCodeDir"] = self.opticks_code_dir
        ivars["OpticksBinariesDir"] = os.path.join(self.opticks_code_dir,
            "Build", "Binaries-solaris-sparc-release")
        ivars["OpticksDependenciesDir"] = \
            self.get_dependencies(self.output_dir)
        ivars["OpticksHelpDir"] = self.get_help()
        ivars["OpticksReleaseDir"] = os.path.abspath(self.opticks_release_dir)
        ivars["currentDir"] = os.path.abspath(".")
        pkg_success = run_pkgmk(ivars, self.output_dir)
        if pkg_success != 0:
            raise ScriptException("Running pkgmk failed with "\
                "an error code of %s" % (pkg_success))

        version = self.get_data_from_pkginfo("VERSION")
        pkgname = self.get_data_from_pkginfo("PKG")
        output_pkg_name = "opticks-%s-sol10-sparc.pkg" % (version)

        if self.verbosity > 1:
            print "Creating datastream format of package..."
        pkgtrans_args = list()
        pkgtrans_args.append("pkgtrans")
        pkgtrans_args.append("-s")
        pkgtrans_args.append(self.output_dir)
        pkgtrans_args.append(output_pkg_name)
        pkgtrans_args.append(pkgname)

        retcode = execute_process(pkgtrans_args)
        if retcode != 0:
            raise ScriptException("Running pkgtrans failed with "\
                "an error code of %s" % (retcode))
        if self.verbosity > 1:
            print "Done creating datastream format of package"

        if self.package_dir is not None and os.path.exists(self.package_dir):
            output_tar_bz2 = os.path.abspath(os.path.join(self.package_dir,
                "opticks-%s-sol10-sparc.tar.bz2" % (version)))
            if self.verbosity > 1:
                print "Creating compressed archive at %s..." % (output_tar_bz2)
            tar_input = os.path.abspath(os.path.join(self.output_dir,
                "tar-input"))
            if os.path.exists(tar_input):
                shutil.rmtree(tar_input, True)
            os.makedirs(tar_input)
            shutil.copy2(os.path.abspath(os.path.join(self.output_dir,
                output_pkg_name)), os.path.join(tar_input, output_pkg_name))
            shutil.copy2(os.path.abspath(os.path.join("DownloadDocs",
                "SolPackage", "INSTALLING.txt")),
                os.path.join(tar_input, "INSTALLING.txt"))

            tar_args = list()
            tar_args.append("tar")
            tar_args.append("-cvf")
            tar_args.append("-")
            tar_args.append(".")
            tar = subprocess.Popen(tar_args,
                stdout=subprocess.PIPE, cwd=tar_input)

            output_handle = open(output_tar_bz2, "wb")
            bzip2_args = list()
            bzip2_args.append("bzip2")
            bzip2_args.append("-c")
            bzip2 = subprocess.Popen(bzip2_args,
                stdin=tar.stdout, stdout=output_handle)

            tar_ret = tar.wait()
            bzip_ret = bzip2.wait()
            output_handle.close()
            if tar_ret != 0:
                raise ScriptException("Running tar failed with an error "\
                    "code of %s" % (tar_ret))
            if bzip_ret != 0:
                raise ScriptException("Running bzip2 failed with an "\
                    "error code of %s" % (bzip_ret))

            shutil.rmtree(tar_input, True)
            if self.verbosity > 1:
                print "Done creating compressed archive"
        if self.verbosity > 1:
            print "Done generating Solaris package"

    def get_dependencies(self, output_path):
        dependencies_output = os.path.join(output_path, "Dependencies")
        if not(os.path.exists(dependencies_output)):
            if self.verbosity > 1:
                print "Gathering dependency libraries..."
            sys.path.append(self.opticks_code_dir)
            import commonutils

            dependencies_list = \
                commonutils.get_dependencies(self.opticks_dependencies_dir,
                "Solaris", False, None)
            commonutils.copy_dependencies(dependencies_list,
                dependencies_output)
            if self.verbosity > 1:
                print "Done gathering dependency libraries"
        return dependencies_output

    def get_help(self):
        help_output = os.path.join(self.output_dir, "Help")
        if not(os.path.exists(help_output)):
            help_zip_path = os.path.join(self.opticks_release_dir,
                "Help", "Opticks.zip")
            total_help_output = os.path.join(help_output, "Opticks")
            if self.verbosity > 1:
                print "Unpacking Help located at %s to "\
                    "%s..." % (help_zip_path, total_help_output)
            unzip_file(help_zip_path, total_help_output)
            if self.verbosity > 1:
                print "Done unpacking Help"

        help_prototype_path = os.path.join(self.output_dir, "help-prototype")
        if self.verbosity > 1:
            print "Generating help-prototype in %s by traversing "\
                "contents of Help dir at "\
                "%s..." % (help_prototype_path, help_output)
        help_prototype = open(help_prototype_path, "w")
        for dirpath, dirnames, filenames in os.walk(help_output):
            for the_dir in dirnames:
                total_path = os.path.join(dirpath, the_dir)
                total_path = total_path[len(help_output):]
                help_prototype.write("d none $APPDIR/Help%s "\
                    "0755 root $GROUP\n" % (total_path))
            for the_file in filenames:
                total_path = os.path.join(dirpath, the_file)
                total_path = total_path[len(help_output):]
                help_prototype.write("f none $APPDIR/Help%s="\
                    "$OpticksHelpDir%s 0644 root "\
                    "$GROUP\n" % (total_path, total_path))
        if self.verbosity > 1:
            print "Done generating help-prototype"
        return help_output

def is_windows():
    """Determine if this script is executing on the Windows operating system.
    @return: Return True if script is executed on Windows, False otherwise.
    @rtype: L{bool}

    """
    return sys.platform.startswith("win32")

def unzip_file(src_file, dst_dir):
    """Unzip specified zip file to destination dir.  It will create
     directories represented in zip file.

     @param src_file: path of zip file
     @type src_file: L{str}
     @param dst_dir: path of destination directory
     @type dst_dir: L{str}

     The destination directory will be created if necessary.

    """
    if not(os.path.exists(dst_dir)):
        os.makedirs(dst_dir)
    zip_file = zipfile.ZipFile(src_file, "r")
    for file_info in zip_file.infolist():
        the_dir, the_file = os.path.split(file_info.filename)
        if len(the_file) == 0:
            continue
        full_dir = os.path.join(dst_dir, the_dir)
        if not(os.path.exists(full_dir)):
            os.makedirs(full_dir)
        file_contents = zip_file.read(file_info.filename)
        output_file = open(os.path.join(dst_dir, the_dir, the_file), "wb")
        output_file.write(file_contents)
        output_file.close()
        output_file = None
    zip_file.close()
    zip_file = None

def parse_args(args):
    """Use the optparse python module to parse the command-line
    arguments depending on whether the script is run on Windows or Solaris.

    @return: An object with the following attributes, run the script
    with -h to see descriptions:
      - build_32 (Windows Only)
      - build_64 (Windows Only)
      - wix_path (Windows Only)

    """
    if is_windows():
        desc = "Generate .msi packages for 32-bit and 64-bit "\
            "version of Opticks."
        package_name = ".msi"
    else:
        desc = "Generate Solaris package for Opticks."
        package_name = "package"
    parser = optparse.OptionParser(usage="%prog [options]",
                          version="%prog 1.0",
                          description=desc)
    if is_windows():
        parser.add_option("--32", action="store_true", dest="build_32",
              help="Build the .msi for the 32-bit version of Opticks.")
        parser.add_option("--64", action="store_true", dest="build_64",
              help="Build the .msi for the 64-bit version of Opticks.")
        parser.set_defaults(build_32=False, build_64=False)
        default_wix_path = r"C:\Program Files (x86)\Windows "\
            r"Installer XML v3\bin"
        if not(os.path.exists(default_wix_path)):
            default_wix_path = r"C:\Program Files\Windows Installer XML v3\bin"
            if not(os.path.exists(default_wix_path)):
                default_wix_path = None

        parser.add_option("--wix-path", action="store", dest="wix_path",
            help="The path where the Windows "\
            "Installer XML Toolset (WIX) (i.e. wix.sourceforge.net)"\
            "is installed.")
        parser.add_option("--update-help", action="store_true",
            dest="update_help",
            help="Read the contents of the Help.zip and update "\
            "Opticks-Help.wxs based on the contents of the .zip file")
        parser.set_defaults(wix_path=default_wix_path, update_help=False)

    parser.add_option("--clean", action="store_true", dest="clean_output",
          help="Clean any generated installer output")
    parser.add_option("-c", "--code-dir", action="store",
          dest="opticks_code_dir",
          help="The path to the checkout of the Code folder from "\
          "the Opticks trunk.")
    parser.add_option("-d", "--dependencies", action="store",
          dest="opticks_dependencies_dir",
          help="The path to the checkout of the Dependencies folder "\
          "from the OpticksSupport trunk.")
    parser.add_option("-r", "--release-dir", action="store",
          dest="opticks_release_dir",
          help="The path to the checkout of the Release folder from "\
          "the Opticks trunk.")
    parser.set_defaults(clean_output=False, opticks_release_dir="..")
    if is_windows():
        package_default_output = "WixOutput"
        package_format = ".zip's"
    else:
        package_default_output = "PackageOutput"
        package_format = ".tar.bz2's"
    parser.add_option("--update-version", action="store", dest="version",
        help="Update the appropriate files and then exit.  This "\
        "will cause the next run of this script to "\
        "generate a " + package_name +
        " with the provided version. This version should be major.minor. "\
        "You can include text in this version number but the string must "\
        "be less than 256 characters. You can use the special version of "\
        "'current' which will query the application version #.")
    parser.add_option("--package-dir", action="store", dest="package_dir",
        help="This is the directory where the resulting installers "\
        "should be placed, this directory must already exist.  "\
        "If this is not provided they will be placed under %s.  "\
        "Setting this causes the creation of the %s that can be "\
        "uploaded to the https://opticks.ballforge.net/"\
        "website." % (package_default_output, package_format))
    parser.add_option("-q", "--quiet", help="Print fewer messages",
        action="store_const", dest="verbosity", const=0)
    parser.add_option("-v", "--verbose", help="Print more messages",
        action="store_const", dest="verbosity", const=2)
    parser.set_defaults(verbosity=1)


    options = parser.parse_args(args[1:])[0]

    return options

def main(args):
    """Generate installation packages for Opticks on Windows and Solaris.
    @return: Return zero on success, non-zero on failure.
    @rtype: L{int}

    """

    options = parse_args(args)

    try:
        if is_windows():
            builder = WixBuilder(options.wix_path, options.opticks_code_dir,
                options.opticks_dependencies_dir, options.opticks_release_dir,
                options.package_dir, options.verbosity)
        else:
            builder = PackageBuilder(options.opticks_code_dir,
                options.opticks_dependencies_dir, options.opticks_release_dir,
                options.package_dir, options.verbosity)

        if options.clean_output:
            if options.verbosity > 1:
                print "Cleaning previous output..."
            builder.clean()
            if options.verbosity > 1:
                print "Done cleaning"

        if options.version:
            builder.update_version(options.version)
            return 0

        if is_windows():
            if options.update_help:
                builder.generate_wix_for_help()
                return 0

            if options.build_32:
                builder.generate_installer(False)

            if options.build_64:
                builder.generate_installer(True)
        else:
            builder.generate_installer()
    except Exception, e:
        print "--------------------------"
        traceback.print_exc()
        print "--------------------------"
        return 2000

    return 0

if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
    retcode = main(sys.argv)
    if retcode != 0:
        print "ERROR: Return code is %s" % (retcode)
    sys.exit(retcode)
