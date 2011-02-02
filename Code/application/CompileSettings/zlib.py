import os
import os.path
import SCons.Warnings

class ZlibNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(ZlibNotFound)

def generate(env):
   if env["OS"] == "windows":
       libs = ["zdll"]
   else:
       libs = ["z"]
   env.AppendUnique(LIBS=libs)

def exists(env):
    return env.Detect('Zlib')
