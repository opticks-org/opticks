import os
import os.path
import SCons.Warnings

class OoModtranNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(OoModtranNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "OoModtran")
    if not path:
       SCons.Warnings.warn(OoModtranNotFound,"Could not detect OoModtran")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include" % path,
                        CPPDEFINES=["UNIX"],
                        LIBPATH=['%s/lib/%s' % (path,env['PLATFORM']),
                                 '/usr/local/lib/sparcv9'],
                        LIBS=env.Split("OoModtran fai fsu fui"))

def exists(env):
    return env.Detect('OoModtran')
