import os
import os.path
import SCons.Warnings

class JavaNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(JavaNotFound)

def generate(env):
    path = os.environ.get('JAVA_HOME',None)
    if not path:
       SCons.Warnings.warn(JavaNotFound,"Could not detect Java")
    else:
       if env["PLATFORM"] == "solaris-sparc":
          env.AppendUnique(CXXFLAGS=["-I%s/include" % (path), "-I%s/include/solaris" % (path)],
                           LIBPATH=['%s/jre/lib/sparcv9' % (path), '%s/jre/lib/sparcv9/server' % (path)],
                           LIBS=["java", "jvm"])
       elif env["PLATFORM"] == "linux-x86_64":
          env.AppendUnique(CXXFLAGS=["-I%s/include" % path, "-I%s/include/linux" % path],
                           LIBPATH=["%s/jre/lib/amd64" % path, "%s/jre/lib/amd64/server" % path],
                           LIBS=["jvm"])

def exists(env):
    return env.Detect('java')
