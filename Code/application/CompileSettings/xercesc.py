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
       env.AppendUnique(CXXFLAGS="-I%s/include/%s -I%s/include" % (xerces_path,env["PLATFORM"],xqilla_path),
                        CPPDEFINES="APPLICATION_XERCES",
                        LIBPATH=['%s/lib/%s' % (xerces_path,env["PLATFORM"]),'%s/lib/%s' % (xqilla_path,env["PLATFORM"])],
                        LIBS=['xerces-c','xqilla'])

def exists(env):
    return env.Detect('xercesc')
