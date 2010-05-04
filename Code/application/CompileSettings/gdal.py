import os
import os.path
import SCons.Warnings

class GdalNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(GdalNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    gdalpath = ""
    jpegpath = ""
    if path:
       gdalpath = os.path.join(path, "gdal")
       jpegpath = os.path.join(path, "libjpeg")
    if not gdalpath or not jpegpath:
       SCons.Warnings.warn(GdalNotFound,"Could not detect gdal")
    else:
       env.AppendUnique(CXXFLAGS=["-I%s/include/%s" % (gdalpath,env["PLATFORM"]),
                                  "-I%s/include" % jpegpath],
                        LIBPATH=['%s/lib/%s' % (gdalpath,env["PLATFORM"]),
                                 '%s/lib/%s' % (jpegpath,env["PLATFORM"])],
                        LIBS=["gdal","jpeg"])

def exists(env):
    return env.Detect('gdal')
