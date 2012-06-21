import os
import os.path
import SCons.Warnings

class CgNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(CgNotFound)

def generate(env):
   env.AppendUnique(LIBS=['Cg','CgGL'])

def exists(env):
    return env.Detect('cg')
