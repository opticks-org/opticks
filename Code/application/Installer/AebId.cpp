/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AebId.h"

#include <algorithm>
#include <QtCore/QRegExp>

AebId::AebId()
{
}

AebId::AebId(const std::string& id) : mId(id)
{
   // IDs are case insensitive
   std::transform(mId.begin(), mId.end(), mId.begin(), tolower);
   QRegExp idregexp("^([a-z0-9._%+-]+@[a-z0-9.-]+[.][a-z]{2,4})|((\\{{0,1}([0-9a-f]){8}-([0-9a-f]){4}-([0-9a-f]){4}-([0-9a-f]){4}-([0-9a-f]){12}\\}{0,1}))$");
   if (!idregexp.exactMatch(QString::fromStdString(mId)))
   {
      mId.clear();
   }
   // strip out the {} from a UUID so they compare the same
   if (mId[0] == '{' && mId[mId.size()-1] == '}')
   {
      mId = mId.substr(1, mId.size() - 2);
   }
}

AebId::AebId(const AebId& other) : mId(other.mId)
{
}

AebId::operator std::string() const
{
   return mId;
}

bool AebId::operator==(const AebId& other) const
{
   return mId == other.mId;
}

bool AebId::operator!=(const AebId& other) const
{
   return mId != other.mId;
}

bool AebId::operator<(const AebId& other) const
{
   return mId < other.mId;
}

bool AebId::isValid() const
{
   return !mId.empty();
}

AebId AebId::applicationId()
{
   return AebId("opticks@ballforge.net");
}
