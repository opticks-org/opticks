/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AebVersion.h"
#include "ConfigurationSettings.h"
#include <QtCore/QRegExp>

AebVersion::AebVersion() : mValid(false)
{
}

AebVersion::AebVersion(const std::string& verString) : mValid(false)
{
   QRegExp ver("^(((-?\\d+)|\\*)((([a-zA-Z+_]+)|\\*)(((-?\\d+)|\\*)(([a-zA-Z+_]+)|\\*)?)?)?)(\\.((-?\\d+)|\\*)((([a-zA-Z+_]+)|\\*)(((-?\\d+)|\\*)(([a-zA-Z+_]+)|\\*)?)?)?)*$");
   if (ver.exactMatch(QString::fromStdString(verString)))
   {
      mValid = true;
      mParts = QString::fromStdString(verString).split('.');
   }
}

bool AebVersion::isValid() const
{
   return mValid;
}

bool AebVersion::operator==(const AebVersion& other) const
{
   if (!isValid() && !other.isValid())
   {
      return true;
   }
   if (isValid() != other.isValid())
   {
      return false;
   }
   QStringListIterator it1(mParts);
   QStringListIterator it2(other.mParts);
   // $2$5$8$10
   QRegExp part("(((-?\\d+)|\\*)((([a-zA-Z+_]+)|\\*)(((-?\\d+)|\\*)(([a-zA-Z+_]+)|\\*)?)?)?)");
   while (it1.hasNext() || it2.hasNext())
   {
      QString part1("0"),part2("0");
      if (it1.hasNext())
      {
         part1 = it1.next();
      }
      if (it2.hasNext())
      {
         part2 = it2.next();
      }
      part.indexIn(part1);
      QStringList cap1 = part.capturedTexts();
      part.indexIn(part2);
      QStringList cap2 = part.capturedTexts();
      if (cap1.size() != cap2.size())
      {
         // the same parts must be present for these to be equal
         return false;
      }

      QString a1,a2,b1,b2,c1,c2,d1,d2;
      if (cap1.size() >= 3) a1 = cap1[2];
      if (cap2.size() >= 3) a2 = cap2[2];
      if (cap1.size() >= 6) b1 = cap1[5];
      if (cap2.size() >= 6) b2 = cap2[5];
      if (cap1.size() >= 9) c1 = cap1[8];
      if (cap2.size() >= 9) c2 = cap2[8];
      if (cap1.size() >= 11) d1 = cap1[10];
      if (cap2.size() >= 11) d2 = cap2[10];
      if (a1.isEmpty())
      {
         a1 = "0";
      }
      if (a2.isEmpty())
      {
         a2 = "0";
      }
      if (c1.isEmpty())
      {
         c1 = "0";
      }
      if (c2.isEmpty())
      {
         c2 = "0";
      }

      // A
      if (!a1.isEmpty())
      {
         if ((a1 == "*" && a2 != "*") || (a1 != "*" && a2 == "*"))
         {
            return false;
         }
         if (a1 != "*" && a1.toInt() != a2.toInt())
         {
            return false;
         }
      }
      // B
      if (!b1.isEmpty())
      {
         if (b1 != b2)
         {
            return false;
         }
      }
      // C
      if (!c1.isEmpty())
      {
         if ((c1 == "*" && c2 != "*") || (c1 != "*" && c2 == "*"))
         {
            return false;
         }
         if (c1 != "*" && c1.toInt() != c2.toInt())
         {
            return false;
         }
      }
      // D
      if (!d1.isEmpty())
      {
         if (d1 != d2)
         {
            return false;
         }
      }
   }
   return true;
}

bool AebVersion::operator!=(const AebVersion& other) const
{
   return !operator==(other);
}

bool AebVersion::operator<(const AebVersion& other) const
{
   if (!isValid())
   {
      return false;
   }
   if (!other.isValid())
   {
      return true;
   }
   QStringListIterator it1(mParts);
   QStringListIterator it2(other.mParts);
   // $2$5$8$10
   QRegExp part("(((-?\\d+)|\\*)((([a-zA-Z+_]+)|\\*)(((-?\\d+)|\\*)(([a-zA-Z+_]+)|\\*)?)?)?)");
   while (it1.hasNext() || it2.hasNext())
   {
      QString part1("0"),part2("0");
      if (it1.hasNext())
      {
         part1 = it1.next();
      }
      if (it2.hasNext())
      {
         part2 = it2.next();
      }
      part.indexIn(part1);
      QStringList cap1 = part.capturedTexts();
      part.indexIn(part2);
      QStringList cap2 = part.capturedTexts();

      QString a1,a2,b1,b2,c1,c2,d1,d2;
      if (cap1.size() >= 3) a1 = cap1[2];
      if (cap2.size() >= 3) a2 = cap2[2];
      if (cap1.size() >= 6) b1 = cap1[5];
      if (cap2.size() >= 6) b2 = cap2[5];
      if (cap1.size() >= 9) c1 = cap1[8];
      if (cap2.size() >= 9) c2 = cap2[8];
      if (cap1.size() >= 11) d1 = cap1[10];
      if (cap2.size() >= 11) d2 = cap2[10];
      if (a1.isEmpty())
      {
         a1 = "0";
      }
      if (a2.isEmpty())
      {
         a2 = "0";
      }
      if (c1.isEmpty())
      {
         c1 = "0";
      }
      if (c2.isEmpty())
      {
         c2 = "0";
      }

      // A
      if (!a1.isEmpty())
      {
         if (a1 != "*" && a2 == "*")
         {
            return true;
         }
         if (a1 == "*" && a2 != "*")
         {
            return false;
         }
         if (a1 != "*" && a2 != "*")
         {
            int a1i = a1.toInt();
            int a2i = a2.toInt();
            if (a1i != a2i)
            {
               return a1i < a2i;
            }
         }
      }
      // B
      if (!b1.isEmpty() || !b2.isEmpty())
      {
         if (b1.isEmpty() && !b2.isEmpty())
         {
            return false;
         }
         if (!b1.isEmpty() && b2.isEmpty())
         {
            return true;
         }
         if (b1 != b2)
         {
            return b1 < b2;
         }
      }
      // C
      if (!c1.isEmpty())
      {
         if (c1 != "*" && c2 == "*")
         {
            return true;
         }
         if (c1 == "*" && c2 != "*")
         {
            return false;
         }
         if (c1 != "*" && c2 != "*")
         {
            int c1i = c1.toInt();
            int c2i = c2.toInt();
            if (c1i != c2i)
            {
               return c1i < c2i;
            }
         }
      }
      // D
      if (!d1.isEmpty() || !d2.isEmpty())
      {
         if (d1.isEmpty() && !d2.isEmpty())
         {
            return false;
         }
         if (!d1.isEmpty() && d2.isEmpty())
         {
            return true;
         }
         if (d1 != d2)
         {
            return d1 < d2;
         }
      }
   }
   return false;
}

bool AebVersion::operator<=(const AebVersion& other) const
{
   return operator<(other) || operator==(other);
}

bool AebVersion::operator>(const AebVersion& other) const
{
   return other.operator<(*this);
}

bool AebVersion::operator>=(const AebVersion& other) const
{
   return other.operator<(*this) || operator==(other);
}

std::string AebVersion::toString() const
{
   return mParts.join(".").toStdString();
}

AebVersion AebVersion::appVersion()
{
   return AebVersion(Service<ConfigurationSettings>()->getVersion());
}