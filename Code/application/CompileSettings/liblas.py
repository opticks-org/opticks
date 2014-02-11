import os
import os.path
import SCons.Warnings

class LibLasNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(LibLasNotFound)

def generate(env):
    env.AppendUnique(LIBS=["las","tiff"])

def exists(env):
    return env.Detect('liblas')
