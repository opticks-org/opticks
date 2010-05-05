import os
import os.path
import SCons.Warnings

class YamlCppNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(YamlCppNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "yaml-cpp")
    if not path:
       SCons.Warnings.warn(YamlCppNotFound,"Could not detect yaml-cpp")
    else:
       lib_path = "%s/lib/%s" % (path, env["OPTICKSPLATFORM"])
       if env["OS"] == "windows":
          lib_path = lib_path + "/%s" % env["MODE"]
       env.AppendUnique(CXXFLAGS="-I%s/include" % (path),
                        LIBPATH=[lib_path],
                        LIBS=["yaml-cpp"])

def exists(env):
    return env.Detect('yaml-cpp')
