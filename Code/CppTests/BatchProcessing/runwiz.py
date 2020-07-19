#!/usr/local/bin/python
#
# The information in this file is
# Copyright(c) 2020 Ball Aerospace & Technologies Corporation
# and is subject to the terms and conditions of the
# GNU Lesser General Public License Version 2.1
# The license text is available from
# http://www.gnu.org/licenses/lgpl.html

import common
import sys
import getopt

def main(args):
    wizardfile = ""
    deploymentfile = ""
    bindir = ""

    opts, args = getopt.getopt(args, "c:i:d:")
    for opt, arg in opts:
        if opt == "-c":
            print "Got deployment file"
            deploymentfile = arg
        elif opt == "-d":
            print "Got bin dir"
            bindir = arg
        elif opt == "-i":
            print "Got wizard file"
            wizardfile = arg

    common.runbatch(wizardfile, bindir, deploymentfile)
    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
