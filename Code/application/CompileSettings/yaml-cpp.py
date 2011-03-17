import os
import os.path
import SCons.Warnings

class YamlCppNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(YamlCppNotFound)

def generate(env):
   dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
   env.AppendUnique(CXXFLAGS="-I%s/yaml-cpp" % dep_include,
                    LIBS=["yaml-cpp"])

def exists(env):
    return env.Detect('yaml-cpp')
