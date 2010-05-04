import os
import os.path
import SCons.Warnings

class GlewNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(GlewNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "glew")
    if not path:
       SCons.Warnings.warn(GlewNotFound,"Could not detect glew")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,env['PLATFORM']),
                        LIBPATH=['%s/lib/%s' % (path,env['PLATFORM'])],
                        LIBS=['GLEW'])

def exists(env):
    return env.Detect('glew')
