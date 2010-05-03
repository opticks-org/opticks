import os
import os.path
import SCons.Warnings

class Hdf5NotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(Hdf5NotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "Hdf5")
    if not path:
       SCons.Warnings.warn(Hdf5NotFound,"Could not detect Hdf5")
    else:
       include_platform = env["OPTICKSPLATFORM"]
       hdf5_libs = ["hdf5", "sz"]
       if env["OS"] == "windows":
          include_platform = env["OS"]
          env.AppendUnique(CPPDEFINES=["_HDF5USEDLL_"])
          if env["MODE"] == "release":
             hdf5_libs = ["hdf5dll"]
          else:
             hdf5_libs = ["hdf5ddll"]
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,include_platform),
                        LIBPATH=['%s/lib/%s' % (path,env["OPTICKSPLATFORM"])],
                        LIBS=hdf5_libs)

def exists(env):
    return env.Detect('hdf5')
