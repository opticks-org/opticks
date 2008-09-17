Overview
--------
The zip file containing this README.txt should be used to debug into Opticks
using Visual Studio 2005 SP1. This is useful if you have developed your
plug-ins for Opticks and you are trying to determine if a bug is either in
Opticks or in your plug-in. This does NOT in any way allow you to write
plug-ins for Opticks, it only allows you to debug into the Opticks source
code. The zip file contains the necessary PDB (program database) files to
enable debugging into Opticks either Debug|Win32 or Debug|x64 using Visual
Studio 2005 SP1. The zip file also contains a full copy of the Opticks source
code.

Configuring Visual Studio 2005 SP1
----------------------------------
1) Unzip the zip; let's assume this directory would be C:\Opticks-pdb-source.
2) Open the "Options" dialog in Visual Studio from the "Tools" menu, i.e.
   "Tools\Options...".
3) Open the "Symbols" page under the "Debugging" section in the left-hand tree
   of the Options dialog.
4) Add two new entries to the "Symbol file (.pdb) locations" field.
   4.1) If you unzipped to the location specified in Step 1, the first entry
        would be C:\Opticks-pdb-source\Code\Build\Binaries-Win32-debug\pdbs.
   4.2) If you unzipped to the location specified in Step 1, the second entry
        would be C:\Opticks-pdb-source\Code\Build\Binaries-x64-debug\pdbs.
5) Open the "VC++ Directories" page under the "Projects and Solutions" section
   in the left-hand tree of the Options dialog. 
6) Select "Win32" in the "Platform:" combo box and "Source files" in the
   "Show directories for:" combo box.
7) Add a new entry.  If you unzipped to the location in Step 1, the entry
   would be C:\Opticks-pdb-source\Code.
8) Select "x64" in the "Platform:" combo box and "Source files" in the
   "Show directories for:" combo box.
9) Add a new entry.  If you unzipped to the location in Step 1, the entry
   would be C:\Opticks-pdb-source\Code.
10) At this point, Visual Studio 2005 SP1 has been configured.  From this
    point forward, if you are debugging your plug-in you can step into any
    Opticks function and view the source code for that function.
