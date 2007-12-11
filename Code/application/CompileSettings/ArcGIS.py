import os
import os.path
import SCons.Warnings

class ArcGISNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(ArcGISNotFound)

def generate(env):
    path = os.environ.get('ARCSDK',None)
    if not path:
       SCons.Warnings.warn(ArcGISNotFound,"Could not detect ArcGIS")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include -I%s/com" % (path,path),
                        LIBPATH=["%s/bin" % path],
                        LIBS=["arcsdk"])
    env.Append(CPPDEFINES=["ESRI_UNIX"])

def exists(env):
    return env.Detect('ArcGIS')
