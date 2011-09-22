/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FUSIONEXCEPTION_H
#define FUSIONEXCEPTION_H

#include <string>
#include <sstream>

class FusionException
{
public:
   /**
    * Creates a FusionException based on a string message
    */
   FusionException(std::string msg) :
      mLine(0),
      mMsg(msg),
      mbVerbose(false)
   {
   }

   /**
    * Creates a FusionException based on a string message, line number, and file name
    *
    * Example of usage: throw FusionException("An error occurred!", __LINE__, __FILE__);
    *
    * @param  msg
    *         The message contained in the exception
    * @param  line
    *         The line number. Should be passed in as the compiler directive __LINE__
    * @param  file
    *         The name of the file where the exception occurred. Should be passed in as
    *         the compiler directive __FILE__
    */
   FusionException(std::string msg, int line, const char* file) :
      mFile(file),
      mLine(line),
      mMsg(msg),
      mbVerbose(false)
   {
      toString(true);
   }

   /**
    * Returns the exception as a string for output.
    *
    * @param  bVerbose
    *         Tells the Exception to be verbose (file, line information) when displaying the string
    * @return The string of the error message, including line number and file name.
    */
   std::string toString(bool bVerbose = true)
   {
      if (mbVerbose != bVerbose)
      {
         mbVerbose = bVerbose;
         std::stringstream sstr;
         sstr << "Recovered from exception";
         if (mbVerbose)
         {
            sstr << " thrown at line " << mLine << " in file " << mFile;
         }
         sstr << ": " << mMsg;
         mText = sstr.str();
      }
      return mText;
   }

private:
   std::string mFile;
   int mLine;
   std::string mMsg;
   std::string mText;
   bool mbVerbose;
};

#endif
