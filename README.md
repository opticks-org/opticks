[![Build Status](https://dev.azure.com/Opticks/Opticks%20CI/_apis/build/status/opticks-org.opticks?branchName=master)](https://dev.azure.com/Opticks/Opticks%20CI/_build/latest?definitionId=1&branchName=master)  __master__

# Opticks
Opticks is an open source remote sensing platform.

Download from the Releases section or from [The Opticks website](https://opticks.org/downloads/opticks/).
The latest [user documentation](https://opticks.org/docs/help/latest/Help/Opticks/) and [SDK documentation](https://opticks.org/docs/sdk/latest/) are also available.

## Key Features
* Imagery and video analysis
* Zoom, pan, rotate spatially large datasets
* Layer GIS features, annotations, results, and other information over your data to provide context
* Many image display controls such as colormap, histogram and transparency
* Support for datasets larger than four gigabytes
* Combine steps using graphical wizards
* Support for processing data in its native interleave of BIP, BSQ or BIL
* Add new capabilities via extension

## Supported File Formats
* NITF 2.0/2.1
* GeoTIFF
* ENVI
* ASPAM/PAR
* CGM
* DTED
* RAW
* JPEG
* GIF
* PNG
* BMP
* ESRI Shapefile
* HDF5
* AVI
* MPEG

## Developer notes and repository structure
Please see the [PlugIn Developer](https://opticks.org/x/CAAY) and [Core Developer](https://opticks.org/x/CgAY) wiki pages for more information.

The repository structure follows a few rules:
- Releases are initially made as a release candidate (_rc1_) for a testing period before a final release is made.
- The main devlopment branch is **master** and represents the current, most volatile development line. It's for all new features big and small. Major and minor (_first and second digit_) releases are made from this branch. (4.12.0, 5.0.0, etc.) Only certain developers have commit access to **master**. If you have a bug fix or feature, you need to create an appropriate branch or fork the repo and make a pull request. 
- **support/*** branches are for less volative development and features. Features and fixes which don't break the ABI should do here first and then get moved to **master** if they are applicable for the next major release. Minor (_third digit_) releases are made from these branches. (4.12.1, etc.)
- **hotfix/*** branches are for emergency hot fixes which occur during testing. These result in additional release candidates. (_rc2_, _rc3_, etc.) Once a final release is made, bug fixes should go in the **support/*** branches.
- **feature/*** and **bugfix/*** branches are for large features slated for the next release or future releases. The difference between the two is purely for convenience and a bug can be fixed in a **feature/*** branch if you'd prefer. This is a convenient way to develop a large feature or collaboratively develop a feature. If you're not sure if a feature should go right into **master** or **support/***, or if it should get a **feature/*** branch, create one of these, branching is cheap.
- Include _Fixes #_ messages and links to issues when committing to branches so they are easy to track. These don't need to be in every commit message, just the final one for a branch.

NOTE: You may see some odities in the commit history, tags, etc. since this repository was originally imported from subversion with a fairly complex branch structure.

## Build and Install
The scons and visual studio projects may remain valid but are not maintained. Builds should use cmake. Instructions are available [here](Code/BUILDING.md).
