import os
import os.path
import SCons.Warnings

class GeotiffNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(GeotiffNotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    env.AppendUnique(CXXFLAGS="-I%s/geotiff" % dep_include,
                     LIBS=['geotiff'])

def exists(env):
    return env.Detect('geotiff')
