import os
import os.path
import SCons.Warnings

class ShapeLibNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(ShapeLibNotFound)

def generate(env):
    lib = "shp"
    if env["OS"] == "windows":
        lib = "shapelib"
    env.AppendUnique(LIBS=[lib])

def exists(env):
    return env.Detect('shapelib')
