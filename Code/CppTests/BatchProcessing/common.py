#!/usr/local/bin/python
#
# The information in this file is
# Copyright(c) 2020 Ball Aerospace & Technologies Corporation
# and is subject to the terms and conditions of the
# GNU Lesser General Public License Version 2.1
# The license text is available from
# http://www.gnu.org/licenses/lgpl.html

import os
import os.path
import sys
import urllib
import subprocess

def runbatch(wiz, bindir, deploymentfile):
    if os.name == "nt":
        opticks_bin_name = "OpticksBatch"
        test_path = "/T:/cppTestData"
        option_prefix = "/"
    else:
        opticks_bin_name = "solBatch"
        test_path = "/TestData/cppTestData"
        option_prefix = "-"

    script_dir = os.path.abspath(os.path.dirname(sys.argv[0]))
    os.chdir(script_dir)
    data_dir = os.path.join(script_dir, "data")
    if not(os.path.exists(data_dir)):
        os.makedirs(data_dir)
    full_opticks_path = os.path.abspath(os.path.join(bindir,
        "..", "Bin", opticks_bin_name))
    print full_opticks_path
    wiz_path = os.path.join(script_dir, wiz + ".wiz")
    wiz_file = open(wiz_path, "r")

    os.chdir(data_dir)
    new_wiz_path = os.path.join(data_dir, wiz + "-full.wiz")
    new_wiz_file = open(new_wiz_path, "w")
    for line in wiz_file:
        line = line.replace("[[TESTDATA]]", test_path)
        line = line.replace("[[DATADIR]]",
            "/" + urllib.quote(data_dir, ":\\/"))
        new_wiz_file.write(line)

    wiz_file.close()
    new_wiz_file.close()

    batch_args = list()
    batch_args.append(full_opticks_path)
    if deploymentfile:
        batch_args.append("%sdeployment:%s" % (option_prefix, deploymentfile))
    generate_args = batch_args[:]
    generate_args.append("%sgenerate:%s" % (option_prefix, new_wiz_path))
    run_args = batch_args[:]
    run_args.append("%sinput:%s" % (option_prefix,
        os.path.splitext(new_wiz_path)[0] + ".batchwiz"))
    subprocess.call(generate_args)
    subprocess.call(run_args)
