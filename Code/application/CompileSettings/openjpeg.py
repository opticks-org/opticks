import os
import os.path
import SCons.Warnings

class OpenJpegNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(OpenJpegNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "openjpeg")
    if not path:
       SCons.Warnings.warn(OpenJpegFound,"Could not detect openjpeg")
    else:
       openjpeg_lib = "openjpeg"
       if env["OS"] == "windows" and env["MODE"] == "debug":
          openjpeg_lib = openjpeg_lib + "d"
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,env["OS"]),
                        LIBPATH=['%s/lib/%s' % (path,env["OPTICKSPLATFORM"])],
                        LIBS=[openjpeg_lib])

def exists(env):
    return env.Detect('openjpeg')
