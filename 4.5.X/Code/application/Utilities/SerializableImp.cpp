/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SerializableImp.h"

using namespace std;

//  Purpose:    Serialize a string. Convenience function
//  Comments:    Get the string length, and serialize that. Then
//            serialize the string contents.
bool SerializeString(FILE* fp, const string& source)
{
   const unsigned short StringSerializeVersion = 1;
   unsigned int stringLength = source.length();

   fwrite(&StringSerializeVersion, sizeof(StringSerializeVersion), 1, fp);
   fwrite(&stringLength, sizeof(stringLength), 1, fp);
   fwrite(source.data(), stringLength, 1, fp);
   return true;
}

//  Purpose:    Serialize a string. Convenience function
//  Comments:    Get the string length, and serialize that. Then
//            serialize the string contents.
bool DeserializeString(FILE* fp, string& target)
{
   unsigned short StringSerializeVersion;
   unsigned int stringLength;
   char* buffer = NULL;

   fread(&StringSerializeVersion, sizeof(StringSerializeVersion), 1, fp);
   switch (StringSerializeVersion)
   {
   case 1:
      fread(&stringLength, sizeof(stringLength), 1, fp);

      buffer = new char[stringLength+1];
      if (buffer == NULL)
      {
         return false;
      }

      fread(buffer, stringLength, 1, fp);
      buffer[stringLength] = '\0';
      target = buffer;
      delete [] buffer;
      break;

   default:
      return false;
   }

   return true;
}

bool SerializeStringVector(FILE* fp, std::vector<std::string>& aList)
{
   const unsigned short IntrinsicVectorVersion = 1;
   std::string element;
   unsigned int numElements = aList.size();

   fwrite(&IntrinsicVectorVersion, sizeof(IntrinsicVectorVersion), 1, fp);
   fwrite(&numElements, sizeof(numElements), 1, fp);
   for (std::vector<std::string>::iterator itr = aList.begin(); itr != aList.end(); ++itr)
   {
      if (!SerializeString(fp, *itr))
      {
         return false;
      }
   }

   return true;
}

bool DeserializeStringVector(FILE* fp, std::vector<std::string>& theList)
{
   string element;
   unsigned int numElements;
   unsigned int i;
   unsigned short version;
   char* buff = NULL;

   theList.clear();
   fread(&version, sizeof(version), 1, fp);
   switch (version)
   {
   case 1:
      fread(&numElements, sizeof(numElements), 1, fp);
      // See comment about optimization for Serialization. Reverse the
      // logic for deserializing via a buffer.
      for (i = 0; i < numElements; ++i)
      {
         if (!DeserializeString(fp, element))
         {
            return false;
         }

         theList.push_back(element);
      }
      break;
   default:   // Invalid version, not yet handled.
      return false;
   }

   return true;
}
