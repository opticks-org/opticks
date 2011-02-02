import os
import os.path
import SCons.Warnings

class FfmpegNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(FfmpegNotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    if env["OS"] == "windows":
        env.AppendUnique(CPPDEFINES=["EMULATE_INTTYPES", "_FILE_OFFSET_BITS=64", "_LARGEFILE_SOURCE", "_ISOC9X_SOURCE", "MSVC8", "BUILD_SHARED_AV"])
    env.AppendUnique(CXXFLAGS=["-I%s/ffmpeg" % dep_include],
                     LIBS=['avformat','avcodec','avutil'])

def exists(env):
    return env.Detect('ffmpeg')
