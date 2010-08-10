import os
import os.path
import SCons.Warnings

class CfitsioNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(CfitsioNotFound)

def generate(env):
    path = os.path.join(os.environ.get('OPTICKSDEPENDENCIES',None))
    if path:
       path = os.path.join(path, "cfitsio")
    if not path:
       SCons.Warnings.warn(CfitsioNotFound,"Could not detect cfitsio")
    else:
       env.AppendUnique(CXXFLAGS=["-I%s/include" % (path,)],
                        LIBPATH=['%s/lib/%s' % (path,env["OPTICKSPLATFORM"])],
                        LIBS=["cfitsio"])

def exists(env):
    return env.Detect('cfitsio')
