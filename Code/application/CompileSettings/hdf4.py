import os
import os.path
import SCons.Warnings

class Hdf4NotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(Hdf4NotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "Hdf4")
    if not path:
       SCons.Warnings.warn(Hdf4NotFound,"Could not detect Hdf4")
    else:
       include_platform = env["OPTICKSPLATFORM"]
       hdf_libs = ["mfhdf", "df", "jpeg"]
       if env["OS"] == "windows":
          include_platform = env["OS"] 
          if env["MODE"] == "release":
             hdf_libs = ["hm423m", "hd423m"]
          else:
             hdf_libs = ["hm423md", "hd423md"]
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,include_platform),
                        LIBPATH=['%s/lib/%s' % (path,env["OPTICKSPLATFORM"])],
                        LIBS=hdf_libs)



def exists(env):
    return env.Detect('hdf4')
