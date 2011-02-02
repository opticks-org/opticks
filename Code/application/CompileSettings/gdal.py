import os
import os.path
import SCons.Warnings

class GdalNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(GdalNotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    libs = ["gdal", "jpeg"]
    if env["OS"] == "windows":
        libs = ["gdal_i"]
    env.AppendUnique(CXXFLAGS=["-I%s/gdal" % dep_include],
                     LIBS=[libs])

def exists(env):
    return env.Detect('gdal')
