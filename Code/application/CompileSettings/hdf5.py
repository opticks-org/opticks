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
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,env["PLATFORM"]),
                        LIBPATH=['%s/lib/%s' % (path,env["PLATFORM"])],
                        LIBS=["hdf5","sz","z"])

def exists(env):
    return env.Detect('hdf5')
