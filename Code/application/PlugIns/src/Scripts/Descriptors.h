/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <vector>
#include <string>

struct Descriptor
{
   std::string mScriptPath;
   std::string mPlugInName;
   std::string mMenuLocation;
   std::string mVersion;
   std::string mProductionStatus;
   std::string mCreator;
   std::string mCopyright;
   std::string mShortDescription;
   std::string mDescription;
};

extern std::vector<Descriptor> gDescriptors;

void populateDescriptors(const char *pScriptPath, std::vector<Descriptor>& descriptors);

#endif   // DESCRIPTORS_H

 
