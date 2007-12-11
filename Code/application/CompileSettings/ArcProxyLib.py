import os
import os.path
import SCons.Warnings

class ArcProxyNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(ArcProxyNotFound)

def generate(env):
    env.AppendUnique(CXXFLAGS="-I../../ArcProxyLib -IArcProxyLib")

def exists(env):
    return env.Detect('ArcProxy')
