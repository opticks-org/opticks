import os
import os.path
import SCons.Warnings

class GlewNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(GlewNotFound)

def generate(env):
   lib = "GLEW"
   if env["OS"] == "windows":
      lib = "glew32"
      if env["MODE"] == "debug":
          lib = "glew32d"
   env.AppendUnique(LIBS=[lib])

def exists(env):
    return env.Detect('glew')
