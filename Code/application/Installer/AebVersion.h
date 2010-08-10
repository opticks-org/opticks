/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AEBVERSION_H__
#define AEBVERSION_H__

#include <string>
#include <QtCore/QStringList>

class AebVersion
{
public:
   AebVersion();
   AebVersion(const std::string& verString);

   bool isValid() const;
   bool operator==(const AebVersion& other) const;
   bool operator!=(const AebVersion& other) const;
   bool operator<(const AebVersion& other) const;
   bool operator<=(const AebVersion& other) const;
   bool operator>(const AebVersion& other) const;
   bool operator>=(const AebVersion& other) const;

   std::string toString() const;

   static AebVersion appVersion();

private:
   bool mValid;
   QStringList mParts;
};

#endif