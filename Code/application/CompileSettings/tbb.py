import os
import os.path
import SCons.Warnings

class TbbNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(TbbNotFound)

def generate(env):
    libs = ["tbb",
            "tbbmalloc",
            "tbbmalloc_proxy"]
    if env["OS"] == "windows" and env["MODE"] == "debug":
            libs = map(lambda x: x + "_debug", libs)
    env.AppendUnique(LIBS=[libs])

def exists(env):
    return env.Detect('tbb')
