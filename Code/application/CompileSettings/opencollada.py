import os
import os.path
import SCons.Warnings

class OpenColladaNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(OpenColladaNotFound)

def generate(env):
    dep_include = env["OPTICKSDEPENDENCIESINCLUDE"]
    collada_lib = ["OpenCOLLADABaseUtils", "OpenCOLLADAFramework", "OpenCOLLADASaxFrameworkLoader", "OpenCOLLADAStreamWriter", "GeneratedSaxParser", "MathMLSolver", "pcre", "xml", "UTF", "ftoa", "buffer"]
    if env["MODE"] == "debug":
        collada_lib = ["OpenCOLLADABaseUtilsd", "OpenCOLLADAFrameworkd", "OpenCOLLADASaxFrameworkLoaderd", "OpenCOLLADAStreamWriterd", "GeneratedSaxParserd", "MathMLSolverd", "pcred", "xmld", "UTFd", "ftoad", "bufferd"]
    env.AppendUnique(CXXFLAGS="-I%s/opencollada" % dep_include,
                     LIBS=collada_lib)

def exists(env):
    return env.Detect('opencollada')
