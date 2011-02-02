import os
import os.path
import SCons.Warnings

class CfitsioNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(CfitsioNotFound)

def generate(env):
    env.AppendUnique(LIBS=["cfitsio"])

def exists(env):
    return env.Detect('cfitsio')
