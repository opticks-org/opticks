import os
import os.path
import SCons.Warnings

class BoostNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(BoostNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "Boost")
    if not path:
       SCons.Warnings.warn(BoostNotFound,"Could not detect Boost")
    else:
       env.AppendUnique(CXXFLAGS="-I%s" % (path))

def exists(env):
    return env.Detect('boost')
