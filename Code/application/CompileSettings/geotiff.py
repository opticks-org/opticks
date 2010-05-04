import os
import os.path
import SCons.Warnings

class GeotiffNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(GeotiffNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "libgeotiff")
    if not path:
       SCons.Warnings.warn(GeotiffNotFound,"Could not detect Geotiff")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,env['PLATFORM']),
                        LIBPATH=['%s/lib/%s' % (path,env['PLATFORM'])],
                        LIBS=['geotiff'])

def exists(env):
    return env.Detect('geotiff')
