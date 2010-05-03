import os
import os.path
import SCons.Warnings

class ArcGISNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(ArcGISNotFound)

def generate(env):
    if env["OS"] != "windows":
       SCons.Warnings.warn(ArcGISNotFound,"ArcGIS only supported on Windows platform")

    path = os.environ.get('ARCSDK',None)
    if not path:
       SCons.Warnings.warn(ArcGISNotFound,"Could not detect ArcGIS")
    else:
       env.AppendUnique(CXXFLAGS=["-I%s/include" % (path), "-I%s/com" % (path)])
       if env["OS"] == "windows":
          env.AppendUnique(CXXFLAGS=["/wd4192", "/wd4278"])
          env.Append(CPPDEFINES=["ESRI_WINDOWS"])

def exists(env):
    return env.Detect('ArcGIS')
