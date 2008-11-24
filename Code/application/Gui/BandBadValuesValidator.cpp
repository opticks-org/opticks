/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QRegExp>
#include <QtCore/QStringList>

#include "BandBadValuesValidator.h"

using namespace std;

BandBadValuesValidator::BandBadValuesValidator(QObject* parent) :
   QValidator(parent)
{
}

BandBadValuesValidator::~BandBadValuesValidator()
{
}

QValidator::State BandBadValuesValidator::validate(QString& input, int& pos) const
{
   QString pattern = QString("(\\d|\\s|-|,)*");
   QRegExp* pRegExp = new QRegExp(pattern);
   if (pRegExp->exactMatch(input))
   {
      return QValidator::Intermediate;
   }

   return QValidator::Invalid;
}

QString BandBadValuesValidator::convertVectorToString(const vector<int>& vec)
{
   QStringList lstOfNumberStrings;
   QString numberString;
   vector<int>::const_iterator iter = vec.begin();
   vector<int>::const_iterator end = vec.end();
   while (iter != end)
   {
      numberString = QString::number(*iter);
      lstOfNumberStrings.push_back(numberString);
      iter++;
   }
   QString finalString;
   finalString = lstOfNumberStrings.join(QString(","));
   return finalString;
}

const std::vector<int> BandBadValuesValidator::convertStringToVector(QString& input, bool & result)
{
   QStringList lstOfUnparsedStrings;
   lstOfUnparsedStrings = input.split(QString(","), QString::SkipEmptyParts);
   QStringList::iterator iter = lstOfUnparsedStrings.begin();
   QStringList::iterator end = lstOfUnparsedStrings.end();
   bool convertedAll = true;

   vector<int> lstOfInts;
   bool convertedNumber;
   QString unparsedString;
   int intValue;
   while (iter != end)
   {
      unparsedString = *iter;
      intValue = unparsedString.toInt(&convertedNumber, 10);
      if (!convertedNumber)
      {
         convertedAll = false;
         break;
      }

      lstOfInts.push_back(intValue);
      
      iter++;
   }

   result = convertedAll;
   if (convertedAll)
   {
      return lstOfInts;
   }
   else
   {
      lstOfInts.clear();
      return lstOfInts;
   }
}
