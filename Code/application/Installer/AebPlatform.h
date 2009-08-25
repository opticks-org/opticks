/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AEBPLATFORM_H
#define AEBPLATFORM_H

#include <string>

class AebPlatform
{
public:
   AebPlatform();
   AebPlatform(const std::string& platform);
   AebPlatform(const AebPlatform& other);
   bool isValid() const;
   bool operator==(const AebPlatform& other) const;
   bool operator!=(const AebPlatform& other) const;
   std::string toString() const;

private:
   std::string mPlatform;
   static std::string currentPlatform();
};

#endif