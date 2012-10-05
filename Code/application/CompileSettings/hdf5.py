import os
import os.path
import SCons.Warnings

class Hdf5NotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(Hdf5NotFound)

def generate(env):
    hdf5_libs = ["hdf5", "sz"]
    if env["OS"] == "windows":
        env.AppendUnique(CPPDEFINES=["_HDF5USEDLL_"])
        if env["MODE"] == "release":
            hdf5_libs = ["hdf5dll"]
        else:
            hdf5_libs = ["hdf5ddll"]
    env.AppendUnique(LIBS=hdf5_libs)

def exists(env):
    return env.Detect('hdf5')
