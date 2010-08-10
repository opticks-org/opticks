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
       include_platform = env["OPTICKSPLATFORM"]
       libs = ["gdal", "jpeg"]
       if env["OS"] == "windows":
          include_platform = env["OS"]
          libs = ["gdal_i"]
       env.AppendUnique(CXXFLAGS=["-I%s/include/%s" % (gdalpath,include_platform),
                                  "-I%s/include" % jpegpath],
                        LIBPATH=['%s/lib/%s' % (gdalpath,env["OPTICKSPLATFORM"]),
                                 '%s/lib/%s' % (jpegpath,env["OPTICKSPLATFORM"])],
                        LIBS=[libs])

def exists(env):
    return env.Detect('gdal')
