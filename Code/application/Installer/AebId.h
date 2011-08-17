/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AEBID_H
#define AEBID_H

#include <string>

class AebId
{
public:
   AebId();
   AebId(const std::string &id);
   AebId(const AebId &other);
   operator std::string() const;
   bool operator==(const AebId& other) const;
   bool operator!=(const AebId& other) const;
   bool operator<(const AebId& other) const;
   bool isValid() const;

   static AebId applicationId();

private:
   std::string mId;
};

#endif
