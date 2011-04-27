Overview
--------
The zip file containing this README.txt should be used to debug into Opticks
using Visual Studio 2010. This is useful if you have developed your
plug-ins for Opticks and you are trying to determine if a bug is either in
Opticks or in your plug-in. This does NOT in any way allow you to write
plug-ins for Opticks, it only allows you to debug into the Opticks source
code. The zip file contains the necessary PDB (program database) files to
enable debugging into Opticks either Debug|Win32 or Debug|x64 using Visual
Studio 2010. The zip file also contains a full copy of the Opticks source
code.

Configuring Visual Studio 2010
------------------------------
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
5) Click the "Load all symbols" button on this page.
6) Open Opticks.sln in Visual Studio and then display the Property Manager
   window from the "View" menu, i.e. "View\Property Manager".
7) Open the "AnnotationImagePalette" item and then the "Debug | Win32" item.
   Right-click on the "Microsoft.Cpp.Win32.user" file and click on "Properties".
8) Open the "VC++ Directories" page under the "Common Properties" section
   in the left-hand tree of the properties dialog. 
9) Edit the "Source Directories" entry by adding a new entry. 
   If you unzipped to the location in Step 1, the entry
   would be C:\Opticks-pdb-source\Code.
10) Open the "AnnotationImagePalette" item and then the "Debug | x64" item.
    Right-click on the "Microsoft.Cpp.x64.user" file and click on "Properties".
11) Open the "VC++ Directories" page under the "Common Properties" section
    in the left-hand tree of the properties dialog. 
12) Edit the "Source Directories" entry by adding a new entry. 
    If you unzipped to the location in Step 1, the entry
    would be C:\Opticks-pdb-source\Code.
13) At this point, Visual Studio 2010 has been configured.  From this
    point forward, if you are debugging your plug-in you can step into any
    Opticks function and view the source code for that function.
