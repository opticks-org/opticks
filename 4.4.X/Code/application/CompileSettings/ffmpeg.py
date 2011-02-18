import os
import os.path
import SCons.Warnings

class FfmpegNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(FfmpegNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "ffmpeg")
    if not path:
       SCons.Warnings.warn(FfmpegFound,"Could not detect ffmpeg")
    else:
       ppath = "%s/%s" % (path, env["OPTICKSPLATFORM"])
       lib_path = ['%s/libavcodec' % ppath, '%s/libavformat' % ppath, '%s/libavutil' % ppath] 
       if env["OS"] == "windows":
          ppath = "%s/%s" % (path, env["OS"])
          lib_path = "%s/%s/build/%s/%s" % (path, env["OS"], env["OPTICKSPLATFORM"], env["MODE"])
          env.AppendUnique(CPPDEFINES=["EMULATE_INTTYPES", "_FILE_OFFSET_BITS=64", "_LARGEFILE_SOURCE", "_ISOC9X_SOURCE", "MSVC8", "EMULATE_FAST_INT", "BUILD_SHARED_AV"])
       env.AppendUnique(CXXFLAGS=["-I%s" % (ppath), "-I%s/libavcodec" % (ppath), "-I%s/libavformat" % (ppath), "-I%s/libavutil" % (ppath)],
                        LIBPATH=lib_path,
                        LIBS=['avformat','avcodec','avutil'])

def exists(env):
    return env.Detect('ffmpeg')
