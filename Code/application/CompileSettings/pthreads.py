import os
import os.path
import SCons.Warnings

class PthreadsNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(PthreadsNotFound)

def generate(env):
   if env["OS"] == "windows":
      path = os.environ.get('OPTICKSDEPENDENCIES',None)
      if path:
         path = os.path.join(path, "pthreads")
      if not path:
         SCons.Warnings.warn(PthreadsNotFound,"Could not detect pthreads")
      else:
         env.AppendUnique(CXXFLAGS=["-I%s/include" % (path)],
                          LIBPATH=['%s/lib/%s' % (path, env["OPTICKSPLATFORM"])],
                          LIBS=["pthreadVC2"])
   elif env["OS"] == "linux":
      env.AppendUnique(CXXFLAGS="-pthread")

def exists(env):
    return env.Detect('pthreads')
