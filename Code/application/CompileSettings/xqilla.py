import os
import os.path
import SCons.Warnings

class XqillaNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(XqillaNotFound)

def generate(env):
   xqilla_lib = "xqilla"
   if env["OS"] == "windows":
      if env["MODE"] == "release":
         xqilla_lib = "xqilla22"
      else:
         xqilla_lib = "xqilla22d"
   env.AppendUnique(LIBS=[xqilla_lib])

def exists(env):
    return env.Detect('xqilla')
