import os
import os.path
import SCons.Warnings

class Hdf4NotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(Hdf4NotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    hdf_libs = ["mfhdf", "df", "jpeg"]
    if env["OS"] == "windows":
        hdf_libs = ["hm425m", "hd425m"]
    env.AppendUnique(CXXFLAGS="-I%s/hdf" % dep_include,
                     LIBS=hdf_libs)

def exists(env):
    return env.Detect('hdf4')
