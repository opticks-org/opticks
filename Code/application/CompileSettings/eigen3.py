import os
import os.path
import SCons.Warnings

class Eigen3NotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(Eigen3NotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    env.AppendUnique(CXXFLAGS="-I%s/eigen3" % dep_include)

def exists(env):
    return env.Detect('eigen3')
