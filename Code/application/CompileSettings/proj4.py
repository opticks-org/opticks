import os
import os.path
import SCons.Warnings

class Proj4NotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(Proj4NotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "libproj4")
    if not path:
       SCons.Warnings.warn(Proj4Found,"Could not detect Proj4")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/includes" % (path),
                        LIBPATH=['%s/lib/%s' % (path,env["PLATFORM"])],
                        LIBS=['proj4'])

def exists(env):
    return env.Detect('proj4')
