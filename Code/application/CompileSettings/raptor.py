import os
import os.path
import SCons.Warnings

class RaptorNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(RaptorNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    raptor_path = None
    if path:
       raptor_path = os.path.join(path, "raptor")
    if not raptor_path:
       SCons.Warnings.warn(RaptorsNotFound,"Could not detect raptor")
    else:
       raptor_libs = ["raptor", "expat"]
       if env["OS"] == "windows":
          raptor_libs = ["raptor"]
       env.AppendUnique(CXXFLAGS="-I%s/include" % raptor_path,
                        LIBPATH=['%s/lib/%s' % (raptor_path,env["OPTICKSPLATFORM"])],
                        LIBS=raptor_libs)

def exists(env):
    return env.Detect('raptor')
