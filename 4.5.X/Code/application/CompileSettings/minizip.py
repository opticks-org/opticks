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
       minizip_lib = "minizip"
       if env["OS"] == "windows" and env["MODE"] == "debug":
          minizip_lib = minizip_lib + "d"
       env.AppendUnique(CXXFLAGS="-I%s/include" % (path),
                        LIBPATH=['%s/lib/%s' % (path,env["OPTICKSPLATFORM"])],
                        LIBS=[minizip_lib])

def exists(env):
    return env.Detect('minizip')
