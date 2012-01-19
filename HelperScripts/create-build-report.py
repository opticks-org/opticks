import subprocess
import sys
import os.path
from optparse import OptionParser
import xml.dom.minidom
import sets
import re

class ExecutionError:
   def __init__(self, msg, retcode):
      self.msg = msg
      self.retcode = retcode

def parse_options():
   parser = OptionParser(usage="%prog [options] path-to-trunk start-rev",
                         version="%prog 1.0")
   parser.add_option("--username", action="store", dest="username", default=None)
   parser.add_option("--password", action="store", dest="password", default=None)
   parser.add_option("--end-rev", action="store", dest="endRev", default="HEAD")
   parser.add_option("--report-name", action="store", dest="report_name", default="Default")
   parser.add_option("--ignore-path", action="append", dest="ignoredPaths")
   (options, args) = parser.parse_args()
   options.trunkPath = None
   options.startRev = None
   if (len(args) != 2):
      raise ExecutionError("Need to provide path-to-trunk and start-rev",1)
   options.trunkPath = args[0]
   options.startRev = args[1]

   return options

def run_svn_log(path, startRev, endRev, username, password, file_prefix, name):
   command = list()
   command.append("svn")
   command.append("log")
   command.append("--xml")
   command.append("-r%s:%s" % (startRev, endRev))
   command.append("--verbose")
   if username:
      command.append("--username %s" % (username))
   if password:
      command.append("--password %s" % (password))
   command.append("%s@%s" % (path, startRev))
   output_file_path = "%s-trunk-log.xml" % (file_prefix) 
   output_file= open(output_file_path, "w")
   subprocess.Popen(command, stdout=output_file).wait()
   output_file.close()

   return output_file_path


class LogEntry:
   def __init__(self, revision, author, date, msg):
      self.revision = revision
      self.author = author
      self.date = date
      self.msg = msg

class PathEntry:
   def __init__(self, path, change):
      self.path = path
      self.change = change
   def modified(self):
      return self.change == "M"
   def added(self):
      return self.change == "R" or self.change == "A"
   def removed(self):
      return self.change == "D"

class SvnLog:
   def __init__(self, modified, added, removed, logEntries):
      self.modified = modified
      self.added = added
      self.removed = removed
      self.logEntries = logEntries

# Convert svn log structure from
#  logentry1
#    path1 - added 
#    path2 - deleted
#    path3 - modified
#  logentry2
#    path3 - modified
#  logentry3
#    path4 - deleted
# into the following
#  paths added
#     path1 - (logentry1)
#  paths deleted
#     path2 - (logentry1)
#     path4 - (logentry4)
#  paths modified
#     path3 - (logentry1, logentry2)

def parse_log_entry(logentryelem):
   rev = logentryelem.getAttribute("revision")
   msgnodes = logentryelem.getElementsByTagName("msg")
   if len(msgnodes) == 1:
      msg = msgnodes[0].firstChild.nodeValue
   authornodes = logentryelem.getElementsByTagName("author")
   if len(authornodes) == 1:
      author = authornodes[0].firstChild.nodeValue
   datenodes = logentryelem.getElementsByTagName("date")
   if len(datenodes) == 1:
      date = datenodes[0].firstChild.nodeValue
   logEntry = LogEntry(rev, author, date, msg)
   return logEntry

def parse_path_entry(pathelem):
   change = pathelem.getAttribute("action")
   path = pathelem.firstChild.nodeValue
   return PathEntry(path, change)
   
def parse_svn_log(filename):
   doc = xml.dom.minidom.parse(filename)
   root = doc.documentElement #get root node
   modifiedPathEntries = dict()
   addedPathEntries = dict()
   removedPathEntries = dict()
   logEntries = list()
   for logelem in root.getElementsByTagName("logentry"):
      pathnodes = logelem.getElementsByTagName("paths")
      logEntry = parse_log_entry(logelem)
      logEntries.append(logEntry)
      if len(pathnodes) == 1:
         for pathelem in pathnodes[0].getElementsByTagName("path"):
            pathEntry = parse_path_entry(pathelem)
            #now group them
            correctDict = None
            if pathEntry.added():
               correctDict = addedPathEntries
            elif pathEntry.removed():
               correctDict = removedPathEntries
            elif pathEntry.modified():
               correctDict = modifiedPathEntries
            else:
               #This is an error
               raise ExecutionError("Unrecognized change %s made to file on trunk, failing" % (pathEntry.change), 1) 
            (path, file) = os.path.split(pathEntry.path)
            if not(correctDict.has_key(path)):
               #not already present create dict to hold file entries
               correctDict[path] = dict()
               fileDict = correctDict[path]
            else:
               fileDict = correctDict[path]
            if not(fileDict.has_key(file)):
               fileDict[file] = list()
            fileDict[file].append(logEntry)
   return SvnLog(modifiedPathEntries, addedPathEntries, removedPathEntries, logEntries)

def trim_added_entries(log):
   #create all dir entries, sorted
   addedPaths = log.added.keys()
   addedPaths.sort()
   dirs = list()
   for path in addedPaths:
      addedFiles = log.added[path].keys()
      addedFiles.sort()
      for file in addedFiles:
         dirs.append(path + "/" + file) 
   #now iterate and mark any we find at the top level, ie.
   #keep in list
   minDirs = list()
   for dir in dirs:
      if log.added.has_key(dir):
         #found at top level, so keep
         minDirs.append(dir)
   dirs = None
   #now remove entries
   for dir in minDirs:
      #print dir
      (basedir, lastDir) = os.path.split(dir)
      pathDict = log.added[basedir]
      del pathDict[lastDir]

def split_branch_name_and_msg(msg):
   restOfMsg = None
   colonPos = msg.find(":") 
   if colonPos != -1:
      branchName = msg[:colonPos]
      restOfMsg = msg[colonPos+1:]
   else:
      #Print a warning here
      branchName = "UNKNOWN"
      restOfMsg = msg
   return (branchName, restOfMsg)

def create_warning_report(log, output):
   #convert lists into sets
   def convert_to_set(pathDict):
      paths = pathDict.keys()
      pathSet = sets.Set()
      for path in paths:
         for file in pathDict[path]:
            fullPath = path + "/" + file
            pathSet.add(fullPath)
      return pathSet
   def output_warning(set, msg):
      if len(set) > 0:
         for path in set:
            output.write("<li>%s was %s</li>" % (path, msg))


   addedSet = convert_to_set(log.added)
   removedSet = convert_to_set(log.removed)
   modifiedSet = convert_to_set(log.modified)

   #determine if there is any intersection, if so print warning
   addRemoved = addedSet.intersection(removedSet)
   addModified = addedSet.intersection(modifiedSet)
   removeModified = removedSet.intersection(modifiedSet)
   output.write("<h3>Warnings</h3>")
   output.write("<ul>")
   output_warning(addRemoved, "added and removed")
   output_warning(addModified, "added and modified")
   output_warning(removeModified, "removed and modified")
   output.write("</ul>")


def create_html_build_report(log, ignoredPaths, report_name, svn_url, svn_start_rev, svn_end_rev, output_file):
   def print_table(name, paths):
      output.write("<h2>%s</h2>" % (name))
      output.write("<table border='1' cellspacing='1'><tr><th>Filename</th><th>Branches</th></tr>")
      
      sortedPaths = paths.keys()
      sortedPaths.sort()
      if ignoredPaths:
         for ignoredPath in ignoredPaths:
            removePaths = list() 
            for path in sortedPaths:
               if path.startswith(ignoredPath):
                  removePaths.append(path)
            for removePath in removePaths:
               sortedPaths.remove(removePath)

      for path in sortedPaths:
         files = paths[path].keys()
         files.sort()
         if len(files) > 0:
            output.write("<tr>")
            output.write("<td colspan='2'><b>%s</b></td>" % (path))
            for file in files:
               output.write("<tr>")
               output.write("<td>%s</td>" % (file))
               output.write("<td>")
               labelStr = ""
               for logEntry in paths[path][file]:
                  #determine branch name, ie. everything before : character
                  (branchName, msg) = split_branch_name_and_msg(logEntry.msg) 
                  labelStr = labelStr + branchName + ", " 
               labelStr = labelStr[:-2] #trim trailing ', ' characters
               output.write(labelStr)
               output.write("</td>")
               output.write("</tr>")
      output.write("</table>")
         
   output = open(output_file, "w")
   output.write("<html><head><title>%s Build Report</title></head><body>" % (report_name))
   output.write("<h1>%s Build Report</h1>" % (report_name))
   output.write("<h3>Subversion Information</h3>")
   output.write("<b>Subversion URL:</b> %s<br/>" % (svn_url))
   output.write("<b>Start Revision:</b> %s<br/>" % (svn_start_rev))
   output.write("<b>Stop Revision:</b> %s" % (svn_end_rev))
   if ignoredPaths:
      output.write("<h3>Ignored Paths:</h3>")
      output.write("<ul>")
      for ignoredPath in ignoredPaths:
         output.write("<li>%s</li>" % (ignoredPath))
      output.write("</ul>")

   output.write("<h2>Branches<h2>")
   output.write("<table border='1' cellspacing='1'>"
                "<tr><th>Branch Name</th><th>Merger</th><th>Revision #</th><th>Date Merged In</th><th>Log Message</th></tr>")
   for logEntry in log.logEntries:
      output.write("<tr>")
      (branchName, msg) = split_branch_name_and_msg(logEntry.msg) 
      output.write("<td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td>" % (branchName, logEntry.author, logEntry.revision, logEntry.date, msg))
      output.write("</tr>")
   output.write("</table>")
   print_table("Added Files", log.added)
   print_table("Removed Files", log.removed)
   print_table("Modified Files", log.modified)
   create_warning_report(log, output) 
   output.write("</body></html>")


def main():
   try:
      args = parse_options()
      file_prefix = args.report_name 
      file_prefix = file_prefix.lower()
      file_prefix = re.sub(r"\s+", "-", file_prefix)
      log_file = run_svn_log(args.trunkPath, args.startRev, args.endRev, args.username, args.password, file_prefix, args.report_name)
      log = parse_svn_log(log_file)
      
      trim_added_entries(log)
      create_html_build_report(log, args.ignoredPaths, args.report_name, args.trunkPath, args.startRev, args.endRev, "%s-build-report.html" % (file_prefix))
   except ExecutionError, ex:
      print ex.msg
      sys.exit(ex.retcode)

if __name__ == "__main__":
   main()
