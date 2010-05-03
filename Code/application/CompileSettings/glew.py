import os
import os.path
import SCons.Warnings

class GlewNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(GlewNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "glew")
    if not path:
       SCons.Warnings.warn(GlewNotFound,"Could not detect glew")
    else:
       lib = "GLEW"
       platform = env["OPTICKSPLATFORM"]
       if env["OS"] == "windows":
          lib = "glew32"
          if env["MODE"] == "debug":
             lib = "glew32d"
          platform = ""
       env.AppendUnique(CXXFLAGS="-I%s/include/%s" % (path,platform),
                        LIBPATH=['%s/lib/%s' % (path,env['OPTICKSPLATFORM'])],
                        LIBS=[lib])

def exists(env):
    return env.Detect('glew')
