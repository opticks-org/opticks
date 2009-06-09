import os
import os.path
import SCons.Warnings

class MinizipNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(MinizipNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "minizip")
    if not path:
       SCons.Warnings.warn(MinizipFound,"Could not detect minizip")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include" % (path),
                        LIBPATH=['%s/lib/%s' % (path,env["PLATFORM"])],
                        LIBS=['minizip'])

def exists(env):
    return env.Detect('minizip')
