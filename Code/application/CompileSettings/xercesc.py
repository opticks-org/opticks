import os
import os.path
import SCons.Warnings

class XercesNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(XercesNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    xqilla_path, xerces_path = None,None
    if path:
       xerces_path = os.path.join(path, "Xerces")
       xqilla_path = os.path.join(path, "xqilla")
    if not xerces_path:
       SCons.Warnings.warn(XercesNotFound,"Could not detect Xerces")
    if not xqilla_path:
       SCons.Warnings.warn(XercesNotFound,"Could not detect XQilla")
    else:
       include_platform = env["OPTICKSPLATFORM"]
       xerces_lib = "xerces-c"
       xqilla_lib = "xqilla"
       if env["OS"] == "windows":
          include_platform = env["OS"] 
          if env["MODE"] == "release":
             xerces_lib = "xerces-c_2"
             xqilla_lib = "xqilla10"
          else:
             xerces_lib = "xerces-c_2D"
             xqilla_lib = "xqilla10d"
       env.AppendUnique(CXXFLAGS=["-I%s/include/%s" % (xerces_path,include_platform), "-I%s/include" % (xqilla_path)],
                        CPPDEFINES="APPLICATION_XERCES",
                        LIBPATH=['%s/lib/%s' % (xerces_path,env["OPTICKSPLATFORM"]),'%s/lib/%s' % (xqilla_path,env["OPTICKSPLATFORM"])],
                        LIBS=[xerces_lib,xqilla_lib])

def exists(env):
    return env.Detect('xercesc')
