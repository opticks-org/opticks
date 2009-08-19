import os
import os.path
import SCons.Warnings

class QwtNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(QwtNotFound)

def generate(env):
    path = os.environ.get('OPTICKSDEPENDENCIES',None)
    if path:
       path = os.path.join(path, "qwt")
    if not path:
       SCons.Warnings.warn(QwtNotFound,"Could not detect Qwt")
    else:
       qtpath = os.environ.get('OPTICKSDEPENDENCIES',None)
       if qtpath:
          qtpath = os.path.join(qtpath, "Qt")
       if not qtpath:
          SCons.Warnings.warn(QwtNotFound,"Could not detect Qt")
       else:
          env.AppendUnique(CXXFLAGS=["-I%s/include" % (path), "-I%s/include/%s/QtCore" % (qtpath,env["PLATFORM"]), "-I%s/include/%s/QtGui" % (qtpath,env["PLATFORM"])],
                           LIBPATH=['%s/lib/%s' % (path,env["PLATFORM"])],
                           LIBS=['qwt'])

def exists(env):
    return env.Detect('qwt')
