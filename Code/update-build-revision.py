#!/usr/bin/env python
import subprocess
import os
import os.path
import sys

def main():
    #chdir to the directory where the script resides
    os.chdir(os.path.abspath(os.path.dirname(sys.argv[0])))

    build_rev_file = os.path.join("application", "Utilities",
        "BuildRevision.h")
    existing_build_rev = None
    if os.path.exists(build_rev_file):
        build_rev_handle = open(build_rev_file, "r")
        build_rev_data = build_rev_handle.read()
        build_rev_handle.close()
        build_rev_split = build_rev_data.split()
        if len(build_rev_split) == 3:
            existing_build_rev = build_rev_split[2]
            existing_build_rev = existing_build_rev[1:-1]

    if os.path.exists(".svn") or os.path.exists("_svn"):
        process = subprocess.Popen(["svnversion", "-c", "-n", "."],
            stdout=subprocess.PIPE, stdin=subprocess.PIPE)
        stdout = process.communicate()[0]
        if process.returncode != 0:
            print "Problem running svnversion"
            return 1000
        version_line_split = stdout.split(":")
        if len(version_line_split) != 2:
            print "Unexpected output from svnversion"
            return 1001
        version_number = version_line_split[1]
        if version_number.endswith("S"):
            print "Switched working copy not currently supported"
            return 1002
        if version_number.endswith("M"):
            version_number = version_number[:-1] + "*"
    else:
        version_number = "NoRev"
    if existing_build_rev != version_number:
        build_rev_handle = open(build_rev_file, "w")
        build_rev_handle.write('#define BUILD_REVISION "%s"' % \
            (version_number))
        build_rev_handle.close()
        print "New Build Revision # is", version_number
    else:
        print "Build Revision # %s is up-to-date" % version_number
    return 0

if __name__ == "__main__":
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
    retcode = main()
    sys.exit(retcode)
