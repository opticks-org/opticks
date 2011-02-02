import os
import os.path
import SCons.Warnings

class LibTiffNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(LibTiffNotFound)

def generate(env):
    lib = ["tiff","jpeg"]
    if env["OS"] == "windows":
        lib = ["libtiff"]
    env.AppendUnique(LIBS=lib)

def exists(env):
    return env.Detect('libtiff')
