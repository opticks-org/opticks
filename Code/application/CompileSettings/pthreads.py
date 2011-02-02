import os
import os.path
import SCons.Warnings

class PthreadsNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(PthreadsNotFound)

def generate(env):
   if env["OS"] == "windows":
     env.AppendUnique(LIBS=["pthreadVC2"])
   elif env["OS"] == "linux":
      env.AppendUnique(CXXFLAGS="-pthread")

def exists(env):
    return env.Detect('pthreads')
