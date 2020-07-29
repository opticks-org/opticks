import os
import os.path
import SCons.Warnings

class QwtNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(QwtNotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    lib = "qwt"
    if env["OS"] == "windows":
        lib = "qwt5"
    env.AppendUnique(CXXFLAGS=["-I%s/qwt-qt5" % dep_include, "-I%s/qt5/QtCore" % dep_include, "-I%s/qt5/QtGui" % dep_include],
                     LIBS=[lib])

def exists(env):
    return env.Detect('qwt')
