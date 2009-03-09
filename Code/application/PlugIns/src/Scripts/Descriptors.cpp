/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "Descriptors.h"
#include "FileResource.h"
#include "StringUtilities.h"

#ifdef UNIX_API
#include <strings.h>
#endif

using namespace std;

vector<Descriptor> gDescriptors;
#if defined(WIN_API)
#define STRNICMP _strnicmp
#else
#define STRNICMP strncasecmp
#endif

struct FieldAssociation
{
   FieldAssociation(const char* pFieldName, string& associatedField) :
      mpFieldName(pFieldName),
      mAssociatedField(associatedField),
      mHaveField(false)
   {
   }

   const char* mpFieldName;
   string& mAssociatedField;
   bool mHaveField;
};

static bool populateDescriptor(string filename, Descriptor& descriptor)
{
#if defined(WIN_API)
   const char* pComment = "REM ";
#else
   const char* pComment = "#";
#endif
   FieldAssociation associations[] = 
   {
      FieldAssociation("PLUGIN_NAME", descriptor.mPlugInName),
      FieldAssociation("PLUGIN_MENU_LOCATION", descriptor.mMenuLocation),
      FieldAssociation("PLUGIN_VERSION", descriptor.mVersion),
      FieldAssociation("PLUGIN_PRODUCTION", descriptor.mProductionStatus),
      FieldAssociation("PLUGIN_CREATOR", descriptor.mCreator),
      FieldAssociation("PLUGIN_COPYRIGHT", descriptor.mCopyright),
      FieldAssociation("PLUGIN_SHORT_DESCRIPTION", descriptor.mShortDescription),
      FieldAssociation("PLUGIN_DESCRIPTION", descriptor.mDescription)
   };
   int commentLen = strlen(pComment);
   char lineBuffer[256+4];
   char fieldBuffer[256+4];
   FileResource file(filename.c_str(), "rt");
   bool done = false;
   bool lineContainsField = false;
   while (!done)
   {
      int i = 0;
      lineContainsField = false;
      fgets(lineBuffer, sizeof(lineBuffer) - 1, &*file);
      if (STRNICMP(lineBuffer, pComment, commentLen) == 0)
      {
         string lineString = StringUtilities::stripWhitespace(&lineBuffer[commentLen]);
         for (i = 0; i < sizeof(associations) / sizeof(associations[0]); ++i)
         {
            if (!associations[i].mHaveField)
            {
               int count;
               char scanBuffer[256];
               sprintf(scanBuffer, "%s%s", associations[i].mpFieldName, "%*[^=]=%[^\n]");
               count = sscanf(lineString.c_str(), scanBuffer, fieldBuffer);
               if (count == 1)
               {
                  associations[i].mAssociatedField = StringUtilities::stripWhitespace(fieldBuffer);
                  associations[i].mHaveField = true;
                  lineContainsField = true;
                  break;
               }
            }
         }
      }
      if (lineContainsField)
      {
         for (i = 0; i < sizeof(associations) / sizeof(associations[0]); ++i)
         {
            if (!associations[i].mHaveField)
            {
               break;
            }
         }
         if (i == sizeof(associations) / sizeof(associations[0]))
         {
            done = true;
         }
      }
      if (feof(&*file))
      {
         done = true;
      }
   }

   if (associations[0].mHaveField) // haveName - the only required field
   {
      descriptor.mScriptPath = filename;
   }

   return associations[0].mHaveField; // haveName - the only required field
}
