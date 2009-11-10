import os
import os.path
import SCons.Warnings

class FfmpegNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(FfmpegNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "ffmpeg", env["PLATFORM"])
    if not path:
       SCons.Warnings.warn(FfmpegFound,"Could not detect ffmpeg")
    else:
       env.AppendUnique(CXXFLAGS=["-I%s" % (path), "-I%s/libavcodec" % (path), "-I%s/libavformat" % (path), "-I%s/libavutil" % (path)],
                        LIBPATH=['%s/libavcodec' % path,
                                 '%s/libavformat' % path,
                                 '%s/libavutil' % path],
                        LIBS=['avformat','avcodec','avutil'])

def exists(env):
    return env.Detect('ffmpeg')
