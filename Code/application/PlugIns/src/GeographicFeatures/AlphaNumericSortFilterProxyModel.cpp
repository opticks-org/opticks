/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AlphaNumericSortFilterProxyModel.h"

#include <cctype>
#include <string>

AlphaNumericSortFilterProxyModel::AlphaNumericSortFilterProxyModel(QObject* pParent) :
   QSortFilterProxyModel(pParent)
{}

AlphaNumericSortFilterProxyModel::~AlphaNumericSortFilterProxyModel()
{}


// This does not work for strings which contain numeric values which are not at the end of the text. E.g.:
// "Opticks 4.10.0rc1" will be placed before "Opticks 4.9.0rc2". This should probably be recursive and start at the
// beginning of the string and not the end. Something like http://www.davekoelle.com/alphanum.html is probably closer
// to the functionality we want here.
bool AlphaNumericSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
   // If data is of string type
   QVariant leftData = sourceModel()->data(left);
   QVariant rightData = sourceModel()->data(right);
   if (leftData.type() == QVariant::String)
   {
      // Get string representation of data.
      std::string leftStr = leftData.toString().toStdString();
      std::string rightStr = rightData.toString().toStdString();

      if (leftStr == rightStr)
      {
         return false;
      }

      // find where the numeric suffix begins
      std::string::size_type leftNumericSuffixPos = leftStr.size();
      while (leftNumericSuffixPos > 0 && std::isdigit(leftStr.at(leftNumericSuffixPos - 1)))
      {
         --leftNumericSuffixPos;
      }
      std::string::size_type rightNumericSuffixPos = rightStr.size();
      while (rightNumericSuffixPos > 0 && std::isdigit(rightStr.at(rightNumericSuffixPos - 1)))
      {
         --rightNumericSuffixPos;
      }

      // compare the text prior to the numeric suffix, then compare the suffix
      std::string leftBase = leftStr.substr(0, leftNumericSuffixPos);
      std::string rightBase = rightStr.substr(0, rightNumericSuffixPos);
      if (0 != leftBase.compare(rightBase))
      {
         return (leftBase.compare(rightBase) < 0);
      }
      if (leftBase.length() == leftStr.length())
      {
         return true;
      }
      if (rightBase.length() == rightStr.length())
      {
         return false;
      }
      std::string leftSuffix = leftStr.substr(leftNumericSuffixPos, leftStr.size() - leftNumericSuffixPos);
      std::string rightSuffix = rightStr.substr(rightNumericSuffixPos, rightStr.size() - rightNumericSuffixPos);
      int leftInt = QString::fromStdString(leftSuffix).toInt();
      int rightInt = QString::fromStdString(rightSuffix).toInt();
      return leftInt < rightInt;
   }

   // Default implemenation.
   return QSortFilterProxyModel::lessThan(left, right);
}
