import os
import os.path
import SCons.Warnings

class Proj4NotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(Proj4NotFound)

def generate(env):
    lib = "proj"
    if env["OS"] == "windows":
        lib = "proj"
    env.AppendUnique(LIBS=[lib])

def exists(env):
    return env.Detect('proj4')
