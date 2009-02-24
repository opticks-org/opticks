import os
import os.path
import SCons.Warnings

class OssimNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(OssimNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "ossim")
    if not path:
       SCons.Warnings.warn(OssimNotFound,"Could not detect Ossim")
    else:
       env.AppendUnique(CXXFLAGS=["-I%s/include" % (path), "-I%s/include/ossim" % (path)],
                        LIBPATH=['%s/lib/%s' % (path,env["PLATFORM"])],
                        LIBS=["ossim"])
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "OpenThreads")
    if not path:
       SCons.Warnings.warn(OssimNotFound,"Could not detect OpenThreads")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include" % (path))

def exists(env):
    return env.Detect('ossim')
