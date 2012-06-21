/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FORMATSTRINGPROCESSOR_H
#define FORMATSTRINGPROCESSOR_H

#include <string>
#include <sstream>
#include <cctype>
#include <stdlib.h>

namespace ArcProxyLib
{
   class FormatStringProcessor
   {
   public:
      FormatStringProcessor() : mInFieldNumber(false)
      {
      }
      virtual ~FormatStringProcessor() {};

      std::string getProcessedString()
      {
         if (mInFieldNumber)
         {
            mProcessedString += getFieldValue(atoi(mFieldNumber.c_str()));
            mInFieldNumber = false;
            mFieldNumber.clear();
         }

         return mProcessedString;
      }

      void operator()(char c)
      {
         bool used = false;
         if (mInFieldNumber)
         {
            if (!std::isdigit(c))
            {
               mProcessedString += getFieldValue(atoi(mFieldNumber.c_str()));
               mInFieldNumber = false;
               mFieldNumber.clear();
            }
            else
            {
               used = true;
               mFieldNumber.push_back(c);
            }
         }
         if (!used)
         {
            if (c == '%')
            {
               mInFieldNumber = true;
            }
            else
            {
               mProcessedString.push_back(c);
            }
         }

      }
   protected:
      virtual std::string getFieldValue(int fieldIndex) const = 0;

   private:
      std::string mProcessedString;
      std::string mFieldNumber;
      bool mInFieldNumber;
   };

   class FormatStringPreprocessor
   {
   public:
      FormatStringPreprocessor(std::string &preprocessedString) 
         : mPreprocessedString(preprocessedString), mInField(false),
         mEscaping(false)
      {
      }
      virtual ~FormatStringPreprocessor() {};

      virtual int getFieldIndex(const std::string &fieldName) const = 0;

      void operator()(char c)
      {
         if (mInField)
         {
            if (c == ']')
            {
               int fieldIndex = getFieldIndex(mFieldName);
               std::stringstream preprocessed;
               if (fieldIndex >= 0)
               {
                  preprocessed << "%" << fieldIndex+1;
               }
               else
               {
                  preprocessed << "[Error: " << mFieldName << " not found]";
               }
               mInField = false;
               mFieldName.clear();
               mPreprocessedString += preprocessed.str();
            }
            else
            {
               mFieldName.push_back(c);
            }
         }
         else
         {
            if (!mEscaping)
            {
               if (c == '[')
               {
                  mInField = true;
               }
               else if (c == '\\')
               {
                  mEscaping = true;
               }
               else
               {
                  mPreprocessedString.push_back(c);
                  if (c == '%')
                  {
                     mPreprocessedString.push_back(c); // %'s must be escaped during regular processing
                  }
               }
            }
            else
            {
               mPreprocessedString.push_back(c);
               mEscaping = false;
            }
         }

      }

   private:
      FormatStringPreprocessor& operator=(const FormatStringPreprocessor& rhs);
      std::string &mPreprocessedString;
      std::string mFieldName;
      bool mInField;
      bool mEscaping;

   };
}

#endif
