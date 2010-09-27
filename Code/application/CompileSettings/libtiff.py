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
       include_platform = env["OPTICKSPLATFORM"]
       lib = "tiff"
       if env["OS"] == "windows":
          include_platform = env["OS"]
          lib = "libtiff"
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,include_platform),
                        LIBPATH=['%s/lib/%s' % (path,env["OPTICKSPLATFORM"])],
                        LIBS=[lib])

def exists(env):
    return env.Detect('libtiff')
