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
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,env["PLATFORM"]),
                        LIBPATH=['%s/lib/%s' % (path,env["PLATFORM"])],
                        LIBS=["mfhdf", "df", "jpeg"])



def exists(env):
    return env.Detect('hdf4')
