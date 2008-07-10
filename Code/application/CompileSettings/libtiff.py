import os
import os.path
import SCons.Warnings

class LibTiffNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(LibTiffNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "libtiff")
    if not path:
       SCons.Warnings.warn(LibTiffNotFound,"Could not detect LibTiff")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,env["PLATFORM"]),
                        LIBPATH=['%s/lib/%s' % (path,env["PLATFORM"])],
                        LIBS=['tiff'])

def exists(env):
    return env.Detect('libtiff')
