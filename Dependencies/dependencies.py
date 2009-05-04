#!/usr/bin/env python

import os
import os.path
import time
import sys

toplevel = os.path.abspath(os.curdir)
verbosity = 1

UNZIP = "/usr/bin/unzip"
if sys.platform == "win32":
   UNZIP = os.path.join(toplevel,"win32-progs","unzip.exe")

class DependencyException(Exception):
   def __init__(self, reason):
      Exception.__init__(self, reason)

class Package(object):
   doc=None
   url=None
   downloadLocation=None
   targetDirectory=None

   def __init__(self, url=None, downloadLocation=None, targetDirectory=None, sentinal=None):
      self.url = url
      self.extraUrl = None
      self.extraDownloadLocation = None
      self.extraTargetDirectory = None
      if not os.path.isabs(downloadLocation):
         self.downloadLocation = os.path.join(toplevel, "packages", downloadLocation)
      else:
         self.downloadLocation = downloadLocation
      if not os.path.isabs(targetDirectory):
         self.targetDirectory = os.path.join(toplevel, targetDirectory)
      else:
         self.targetDirectory = targetDirectory
      if not os.path.isabs(sentinal):
         self.sentinal = os.path.join(toplevel, sentinal)
      else:
         self.sentinal = sentinal

   def addExtraPackage(self, url=None, downloadLocation=None, targetDirectory=None):
      self.extraUrl = url
      if not os.path.isabs(downloadLocation):
         self.extraDownloadLocation = os.path.join(toplevel, "packages", downloadLocation)
      else:
         self.extraDownloadLocation = downloadLocation
      if not os.path.isabs(targetDirectory):
         self.extraTargetDirectory = os.path.join(toplevel, targetDirectory)
      else:
         self.extraTargetDirectory = targetDirectory

   def isValid(self):
      if self.downloadLocation == None or self.targetDirectory == None:
         return False
      return True

   def needsUpdate(self, path, other=time.time()):
      "Returns false if the given path exists and was modified more recently than other."
      if other == None:
         return not os.path.exists(path)
      return not (os.path.exists(path) and os.stat(path)[8] >= os.stat(other)[8])

   def updateSentinal(self):
      open(self.sentinal,"w").close()

   def download(self,force=False,all=False):
      if not self.downloadLocation:
         raise DependencyException("Download location must be defined.")
      if force or self.needsUpdate(self.downloadLocation, None):
         if not self.url:
            raise DependencyException("No package URL specified. This package is not available for download. Place %s package URL in the packages directory." % os.path.basename(self.downloadLocation))
         import urllib
         if verbosity > 0: print "Downloading %s -> %s..." % (self.url, self.downloadLocation)
         if not os.path.exists(os.path.dirname(self.downloadLocation)):
            os.makedirs(os.path.dirname(self.downloadLocation))
         urllib.urlretrieve(self.url, self.downloadLocation)
      else:
         if verbosity > 0: print "%s is up to date..." % os.path.basename(self.downloadLocation)

      if not all or not self.extraDownloadLocation:
         return 
      if force or self.needsUpdate(self.extraDownloadLocation, None):
         if not self.extraUrl:
            raise DependencyException("No package URL specified. This package is not available for download. Place %s package URL in the packages directory." % os.path.basename(self.extraDownloadLocation))
         import urllib
         if verbosity > 0: print "Downloading %s -> %s..." % (self.extraUrl, self.extraDownloadLocation)
         if not os.path.exists(os.path.dirname(self.extraDownloadLocation)):
            os.makedirs(os.path.dirname(self.extraDownloadLocation))
         urllib.urlretrieve(self.extraUrl, self.extraDownloadLocation)
      else:
         if verbosity > 0: print "%s is up to date..." % os.path.basename(self.extraDownloadLocation)

   def unpack(self,force=False,all=False):
      if not self.downloadLocation or not self.targetDirectory:
         raise DependencyException("Download location and target directory must be defined.")

      haveExtras = all and self.extraDownloadLocation is not None
      unpackExtras = haveExtras and (force or self.needsUpdate(self.sentinal, self.extraDownloadLocation))

      if force or self.needsUpdate(self.sentinal, self.downloadLocation):
         import subprocess
         if verbosity > 0: print "Unpacking %s to %s..." % (self.downloadLocation,self.targetDirectory)
         if not os.path.exists(self.targetDirectory):
            os.makedirs(self.targetDirectory)
         if os.path.splitext(self.downloadLocation)[1].lower() == '.zip':
            cmd = [UNZIP]
            if verbosity < 2: cmd.append('-q')
            cmd = cmd + ['-o','-d', self.targetDirectory, self.downloadLocation]
            if subprocess.call(cmd) < 0:
               raise DependencyException("Unable to unzip the package")
            self.updateSentinal()
      else:
         if verbosity > 0: print "%s is up to date..." % os.path.basename(self.targetDirectory)

      if not haveExtras:
         return 

      if not self.extraTargetDirectory:
         raise DependencyException("Target directory must be defined.")
          
      if unpackExtras:
         import subprocess
         if verbosity > 0: print "Unpacking %s to %s..." % (self.extraDownloadLocation,self.extraTargetDirectory)
         if not os.path.exists(self.extraTargetDirectory):
            os.makedirs(self.extraTargetDirectory)
         if os.path.splitext(self.extraDownloadLocation)[1].lower() == '.zip':
            cmd = [UNZIP]
            if verbosity < 2: cmd.append('-q')
            cmd = cmd + ['-o','-d', self.extraTargetDirectory, self.extraDownloadLocation]
            if subprocess.call(cmd) < 0:
               raise DependencyException("Unable to unzip the package")
            self.updateSentinal()
      else:
         if verbosity > 0: print "%s is up to date..." % os.path.basename(self.extraTargetDirectory)
         

def buildDepsList(deppath):
      depdirs = []
      for p in deppath:
         depdirs.extend(filter(lambda e: os.path.isdir(e) and "__init__.py" in os.listdir(e), map(lambda d: os.path.join(os.path.abspath(p), d), os.listdir(p))))
      deps = {}
      for dep in depdirs:
         depname = os.path.basename(dep)
         sys.path = [os.path.join(toplevel,dep)] + sys.path
         fname = os.path.join(toplevel,dep,"__init__.py")
         f = open(fname, "r")
         mod = imp.load_module(dep, f, fname, ('','r',imp.PY_SOURCE))
         f.close()
         if 'package' in mod.__dict__:
            if 'targetDirectory' not in mod.package:
               mod.package['targetDirectory'] = toplevel
            if 'sentinal' not in mod.package:
               mod.package['sentinal'] = os.path.join(toplevel, dep, ".sentinal")
            deps[depname] = apply(Package, [], mod.package)
            deps[depname].doc = mod.__doc__
         else:
            deps[depname] = Package()
            deps[depname].doc = mod.__doc__
         if 'extra_package' in mod.__dict__:
            if 'targetDirectory' not in mod.extra_package:
               mod.extra_package['targetDirectory'] = toplevel
            deps[depname].addExtraPackage(*[], **mod.extra_package)

      return deps

if __name__ == '__main__':
   import optparse,imp,sys,os
   parser = optparse.OptionParser(usage="usage: %prog [options] [dependency]...\nBy default, all dependencies are installed. The OPTICKS_DEP_PATH environment variable, if present, will be searched for dependency configurations.")
   parser.add_option("-m","--mode",help="list: list available dependencies -- build: build from source -- package: the default, install from pre-build packages",action="store",choices=("list","build","package"),default="package",dest="mode")
   parser.add_option("-s","--size",help="min: the default, download only parts of the package required for compilation -- all: all parts of the package",action="store",choices=("min","all"),default="min",dest="package")
   parser.add_option("-f","--force",help="Force installation",action="store_true",dest="force",default=False)
   parser.add_option("-q","--quiet",help="Print fewer messages",action="store_true",dest="quiet")
   parser.add_option("-v","--verbose",help="Print more messages",action="store_true",dest="verbose")
   (options,args) = parser.parse_args()
   if options.quiet: verbosity -= 1
   if options.verbose: verbosity += 1
 
   if options.mode == "build":
      raise DependencyException("Building from source not currently supported")

   deppath = filter(lambda x: len(x) > 0, os.environ.get('OPTICKS_DEP_PATH', "").split(os.pathsep))
   deppath.insert(0, toplevel)
   if verbosity > 1:
      print "Searching the following directories for dependency configurations:"
      for d in deppath:
         print "\t%s" % d
   sys.path.extend(deppath)
   deps = buildDepsList(deppath)
   if options.mode == "list":
      print "The following packages are available. Names preceded by a * can not be installed from a binary package.\n"
      for dep in deps:
         package = ""
         if not deps[dep].isValid(): package = "* "
         print "%s%s: %s" % (package,os.path.basename(dep),deps[dep].doc)
      sys.exit(0)
   if len(args) == 0:
      args = deps.keys()
   if options.package == "all":
      all = True
   else:
      all = False
   for pkg in args:
      if pkg not in deps or not deps[pkg].isValid():
         if verbosity >= 0: print "%s does not support package installation. You must build from source." % pkg
         continue
      p = deps[pkg]
      try:
         p.download(options.force, all)
         p.unpack(options.force, all)
      except DependencyException,e:
         print "Unable to install %s: %s" % (pkg,str(e))
