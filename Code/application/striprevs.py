#!/bin/env python

"This script removes PVCS revision tags from files"

import re
import sys
import os.path
import types

# search for // $Header   through the next */
regexp = re.compile(r'//[ \t]*\$Header.*?\*/',re.DOTALL)

def striprevs(source, destination):
   global regexp
   s = None
   if type(source) == types.FileType:
      s = source.read()
   else:
      s = str(source)
   m = regexp.search(s)
   if m == None:
      print "Can't locate log entries."
      destination.write(s)
   elif m.start() != -1 and m.end() != -1:
      destination.write(s[:m.start()]+s[m.end():])

def recurseDirs(arg, dirname, names):
   arg += map(lambda n,d=dirname: os.path.join(d,n), names)

if __name__ == '__main__':
   if len(sys.argv) < 3:
      print 'Usage: %s file [file ...] destination' % sys.argv[0]
      sys.exit(1)
   files = sys.argv[1:-1]
   destination = sys.argv[-1]

   if len(files) > 1 and not os.path.isdir(destination):
      print 'When multiple files are specified, destination must be a directory.'
      sys.exit(2)
   if len(files) == 1 and not (os.path.isdir(destination) or not os.path.exists(destination)):
      print 'Destination already exists.'
      sys.exit(3)

   if len(files) == 1 and not os.path.exists(destination):
      # special case where destination is a filename
      try:
         src,dst = open(files[0],'r'),open(destination,'w')
         striprevs(src,dst)
         src.close()
         dst.close()
      except IOError,err:
         print str(err)
         sys.exit(4)
   else:
      pathsNormalized = 0
      if len(files) == 1 and os.path.isdir(files[0]) and os.path.isdir(destination):
         if not (files[0] == destination):
            print 'You can only run recursively in place.'
            sys.exit(5)
         srcdir = files[0]
         files = []
         os.path.walk(srcdir,recurseDirs,files)
         pathsNormalized = 1
      for f in files:
         d = None
         if pathsNormalized:
            d = f
         else:
            d = os.path.join(destination,os.path.basename(f))
         try:
            src,dst = None,None
	    if (os.path.isdir(f)):
               continue
            if os.path.exists(f) and os.path.exists(d) and (f == d):
               tmp = open(f,'r')
               src = tmp.read()
               tmp.close()
               dst = open(d,'w')
            else:
               src,dst = open(f,'r'),open(d,'w')
            striprevs(src,dst)
            if type(src) == types.FileType:
               src.close()
            dst.close()
         except IOError,err:
            print str(err)
