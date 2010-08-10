import os
import os.path
import SCons.Warnings

class AspamNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(AspamNotFound)

def generate(env):
    path = "$COREDIR/PlugIns/src/Aspam"
    if not path:
       SCons.Warnings.warn(AspamNotFound,"Could not locate Aspam header files")
    else:
       env.AppendUnique(CXXFLAGS="-I%s" % path)

def exists(env):
    return env.Detect('Aspam')
