import os
import os.path
import SCons.Warnings

class OpenCVNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(OpenCVNotFound)

def generate(env):
    libs = ["opencv_core",
            "opencv_imgproc",
            "opencv_ml",
            "opencv_features2d",
            "opencv_video",
            "opencv_objdetect",
            "opencv_calib3d",
            "opencv_flann"]
    if env["OS"] == "windows":
        if env["MODE"] == "debug":
            libs = map(lambda x: x + "220d", libs)
        else:
            libs = map(lambda x: x + "220", libs)
    env.AppendUnique(LIBS=[libs])

def exists(env):
    return env.Detect('opencv')
