/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef ENVIFIELD_H
#define ENVIFIELD_H

#include <string>
#include <vector>

struct EnviField
{
   ~EnviField();

   bool populateFromHeader(const std::string& filename);
   EnviField* find(const std::string& tag) const;

   std::string mTag;
   std::string mValue;
   std::vector<EnviField*> mChildren;
};

#endif // ENVIFIELD_H
