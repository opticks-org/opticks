import os
import os.path
import SCons.Warnings

class ShapeLibNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(ShapeLibNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "ShapeLib")
    if not path:
       SCons.Warnings.warn(ShapeLibNotFound,"Could not detect ShapeLib")
    else:
       lib = "shp"
       if env["OS"] == "windows":
          lib = "shapelib"
       env.AppendUnique( CXXFLAGS="-I%s/include" % (path),
                         LIBPATH=['%s/lib/%s' % (path,env["OPTICKSPLATFORM"])],
                         LIBS=[lib])

def exists(env):
    return env.Detect('shapelib')
