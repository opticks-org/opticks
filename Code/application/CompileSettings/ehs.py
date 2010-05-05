import os
import os.path
import SCons.Warnings

class EhsNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(EhsNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    ehspath,pcrepath,pmepath = "","",""
    if path:
       ehspath = os.path.join(path, "ehs")
       pcrepath = os.path.join(path, "pcre")
       pmepath = os.path.join(path, "pme")
    if not path:
       SCons.Warnings.warn(EhsNotFound,"Could not detect ehs")
    else:
       ehs_lib = "ehs"
       if env["OS"] == "windows" and env["MODE"] == "debug":
          ehs_lib = ehs_lib + "D"
       env.AppendUnique(CXXFLAGS=["-I%s/include" % (ehspath), "-I%s/include" % (pcrepath), "-I%s/include" % (pmepath)],
                        LIBPATH=['%s/lib/%s' % (ehspath,env["OPTICKSPLATFORM"])],
                        LIBS=[ehs_lib])

def exists(env):
    return env.Detect('ehs')
