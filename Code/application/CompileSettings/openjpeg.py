import os
import os.path
import SCons.Warnings

class OpenJpegNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(OpenJpegNotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    env.AppendUnique(CXXFLAGS=["-I%s/openjpeg" % dep_include],
                     LIBS=["openjp2"])

def exists(env):
    return env.Detect('openjpeg')
