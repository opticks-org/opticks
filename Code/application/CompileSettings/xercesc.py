import os
import os.path
import SCons.Warnings

class XercesNotFound(SCons.Warnings.Warning):
    pass
SCons.Warnings.enableWarningClass(XercesNotFound)

def generate(env):
   xerces_lib = "xerces-c"
   if env["OS"] == "windows":
      if env["MODE"] == "release":
         xerces_lib = "xerces-c_3"
      else:
         xerces_lib = "xerces-c_3D"
   env.AppendUnique(CPPDEFINES="APPLICATION_XERCES",
                    LIBS=[xerces_lib])

def exists(env):
    return env.Detect('xercesc')
