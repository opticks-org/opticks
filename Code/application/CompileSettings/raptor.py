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
       env.AppendUnique(CXXFLAGS="-I%s/include" % raptor_path,
                        LIBPATH=['%s/lib/%s' % (raptor_path,env["PLATFORM"])],
                        LIBS=['raptor','expat'])

def exists(env):
    return env.Detect('raptor')
