/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "EnviField.h"
#include "FileResource.h"
#include "StringUtilities.h"

using namespace std;

namespace
{
EnviField* parseEnviField(vector<string>::iterator& lineIterator)
{
   string line = *lineIterator;
   EnviField* pField = new EnviField();

   string::size_type position = line.find("{");
   if (position == string::npos)
   {
      position = line.find("=");
      if (position == string::npos)
      {
         pField->mTag = "";
         pField->mValue = StringUtilities::stripWhitespace(line);

         if (pField->mValue.empty() == true)
         {
            delete pField;
            return NULL;
         }
      }
      else
      {
         pField->mTag = StringUtilities::toLower(StringUtilities::stripWhitespace(line.substr(0, position)));
         pField->mValue = StringUtilities::stripWhitespace(line.substr(position + 1, line.length() - 1));
      }
   }
   else
   {
      string::size_type position2 = line.find("=");
      pField->mTag = StringUtilities::toLower(StringUtilities::stripWhitespace(line.substr(0, position2)));
      for (string::size_type i = 0; i <= position; ++i)
      {
         (*lineIterator)[i] = ' ';
      }

      for ( ; ; ) // escape condition within the loop
      {
         EnviField* pChild = parseEnviField(lineIterator);
         if (pChild != NULL)
         {
            pField->mChildren.push_back(pChild);
         }
         line = *lineIterator;
         position = line.find("}");
         if (position != string::npos)
         {
            pChild->mValue = StringUtilities::stripWhitespace(pChild->mValue.substr(0, pChild->mValue.length() - 1));
            break;
         }
         ++lineIterator;
      }
   }

   return pField;
}
};

EnviField::~EnviField()
{
   for (vector<EnviField*>::iterator iter = mChildren.begin(); iter != mChildren.end(); ++iter)
   {
      EnviField* pField = *iter;
      if (pField != NULL)
      {
         delete pField;
      }
   }
}

bool EnviField::populateFromHeader(const string& filename)
{
   if (filename.empty())
   {
      return false;
   }

   string line;
   int i;

   FileResource pFile(filename.c_str(), "rt");
   if (pFile.get() == NULL)
   {
      return false;
   }

   char pBuffer[2048];
   fgets(pBuffer, 2047, pFile);
   if (strncmp(pBuffer, "ENVI", 4) != 0)
   {
      return false;
   }

   mTag.erase();
   mValue.erase();
   mChildren.clear();

   vector<string> headerText;      // stores the lines in the ENVI header file
   headerText.push_back("ENVI");

   while (feof(pFile) == 0)
   {
      if (fgets(pBuffer, 2047, pFile) == NULL)
      {
         if (feof(pFile) == 0)
         {
            return false;
         }
      }

      // strip trailing whitespace from the string
      i = strlen(pBuffer) - 1;
      if (i > 2047)
      {
         return false;
      }

      int j;
      for (j = i; j >= 0; j--)
      {
         if (static_cast<unsigned char>(pBuffer[j]) < 9)
         {
            return false;
         }
      }

      while ((i >= 0) && isspace(pBuffer[i]))
      {
         pBuffer[i--] = 0;
      }

      if (strlen(pBuffer) == 0)
      {
         continue;
      }

      line = StringUtilities::stripWhitespace(string(pBuffer));
      if (*(line.end() - 1) == ',')
      {
         line = line.substr(0, line.length() - 1);
      }

      headerText.push_back(line);
   }

   // Parse the header text for the field values
   vector<string>::iterator lineIterator = headerText.begin();

   if (lineIterator == headerText.end()) 
   {
      return false;
   }

   // ENVI header must have "ENVI\n" as the first line
   if (*lineIterator != "ENVI")
   {
      return false;
   }

   mTag = *lineIterator;
   mValue = "";

   ++lineIterator;
   for (; lineIterator != headerText.end(); ++lineIterator)
   {
      EnviField* pField = parseEnviField(lineIterator);
      if (pField != NULL)
      {
         mChildren.push_back(pField);
      }
   }

   return true;
}

EnviField* EnviField::find(const string& tag) const
{
   vector<EnviField*>::const_iterator iter;
   for (iter = mChildren.begin(); iter != mChildren.end(); ++iter)
   {
      EnviField* pField = *iter;
      if (pField != NULL)
      {
         if (pField->mTag == tag)
         {
            return pField;
         }
      }
   }

   return NULL;
}
