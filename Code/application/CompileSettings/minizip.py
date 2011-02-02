import os
import os.path
import SCons.Warnings

class MinizipNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(MinizipNotFound)

def generate(env):
   minizip_lib = "minizip"
   if env["OS"] == "windows" and env["MODE"] == "debug":
       minizip_lib = minizip_lib + "d"
   dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
   env.AppendUnique(CXXFLAGS=["-I%s/minizip" % dep_include],
	            LIBS=[minizip_lib])

def exists(env):
    return env.Detect('minizip')
