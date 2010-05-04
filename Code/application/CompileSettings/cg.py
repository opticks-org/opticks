import os
import os.path
import SCons.Warnings

class CgNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(CgNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "Cg")
    if not path:
       SCons.Warnings.warn(CgNotFound,"Could not detect Cg")
    else:
       env.AppendUnique(CXXFLAGS="-I%s/include" % (path,),
                        LIBPATH=['%s/lib/%s' % (path,env['PLATFORM'])],
                        LIBS=['Cg','CgGL'])

def exists(env):
    return env.Detect('cg')
