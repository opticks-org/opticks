import os
import os.path
import SCons.Warnings

class OssimNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(OssimNotFound)

def generate(env):
    ossim_lib = "ossim"
    if env["OS"] == "windows" and env["MODE"] == "debug":
        ossim_lib = ossim_lib + "d"
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    env.AppendUnique(CXXFLAGS=["-I%s/ossim" % dep_include],
                     LIBS=[ossim_lib])

def exists(env):
    return env.Detect('ossim')
