import os
import os.path
import SCons.Warnings

class RaptorNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(RaptorNotFound)

def generate(env):
   raptor_libs = ["raptor", "expat"]
   if env["OS"] == "windows":
       raptor_libs = ["raptor"]
   env.AppendUnique(LIBS=raptor_libs)

def exists(env):
    return env.Detect('raptor')
