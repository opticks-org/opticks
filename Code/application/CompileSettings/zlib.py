import os
import os.path
import SCons.Warnings

class ZlibNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(ZlibNotFound)

def generate(env):
   if env["OS"] == "windows":
      path = os.environ.get('OPTICKSDEPENDENCIES',None)
      if path:
         path = os.path.join(path, "zlib")
      if env["MODE"] == "release":
         libs = ["zlib1"]
      else:
         libs = ["zlib1d"]
      if not path:
         SCons.Warnings.warn(ZlibNotFound,"Could not detect zlib")
      else:
         env.AppendUnique(CXXFLAGS=["-I%s/include" % (path)],
                          LIBPATH=['%s/lib/%s' % (path, env["OPTICKSPLATFORM"])],
                          LIBS=libs)
   else:
      env.AppendUnique(LIBS=["z"])

def exists(env):
    return env.Detect('Zlib')
