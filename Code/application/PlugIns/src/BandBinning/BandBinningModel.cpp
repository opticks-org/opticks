/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtGui/QFont>

#include "BandBinningModel.h"
#include "RasterDataDescriptor.h"

BandBinningModel::BandBinningModel(const RasterDataDescriptor* pDescriptor, QObject* pParent) :
   QAbstractTableModel(pParent),
   mpDescriptor(pDescriptor)
{}

BandBinningModel::~BandBinningModel()
{}

QVariant BandBinningModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
   {
      if (section == 0)
      {
         return QVariant("Bin Number\n(Output Band)");
      }
      else if (section == 1)
      {
         return QVariant("First\nInput Band");
      }
      else if (section == 2)
      {
         return QVariant("Last\nInput Band");
      }
   }

   return QVariant();
}

int BandBinningModel::rowCount(const QModelIndex& parent) const
{
   return static_cast<int>(mGroupedBands.size());
}

int BandBinningModel::columnCount(const QModelIndex& parent) const
{
   return 3;
}

Qt::ItemFlags BandBinningModel::flags(const QModelIndex& index) const
{
   // Only allow first first and last input bands to be edited by the user.
   Qt::ItemFlags flag = QAbstractTableModel::flags(index);
   if (index.isValid())
   {
      int column = index.column();
      if (column == 1 || column == 2)
      {
         flag |= Qt::ItemIsEditable;
      }
   }

   return flag;
}

bool BandBinningModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
   if (index.isValid() == false)
   {
      return false;
   }

   if (role == Qt::EditRole)
   {
      int row = index.row();
      int column = index.column();
      if (row < 0 || column < 0)
      {
         return false;
      }

      QString valueText = value.toString();
      if (valueText.isEmpty())
      {
         return false;
      }

      bool success = false;
      unsigned int originalNumber = valueText.toUInt(&success);
      if (success == false || originalNumber == 0)
      {
         return false;
      }

      // Display is 1-based; storage is 0-based.
      DimensionDescriptor band = mpDescriptor->getOriginalBand(originalNumber - 1);
      if (band.isValid() == false)
      {
         return false;
      }

      // Ignore ordering of the first and last band in the model.
      // Ordering is done after dismissal of the dialog by calling BandBinningUtilities::preprocessGroupedBands().
      if (column == 1)
      {
         mGroupedBands[row].first = band;
      }
      else if (column == 2)
      {
         mGroupedBands[row].second = band;
      }
      else
      {
         return false;
      }

      emit dataChanged(index, index);
      return true;
   }

   return false;
}

QVariant BandBinningModel::data(const QModelIndex& index, int role) const
{
   if (index.isValid() == false)
   {
      return QVariant();
   }

   int row = index.row();
   int column = index.column();
   if (row < 0 || column < 0)
   {
      return QVariant();
   }

   if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
   {
      if (column == 0)
      {
         if (static_cast<unsigned int>(row) < mGroupedBands.size())
         {
            // Display is 1-based; storage is 0-based.
            return QVariant(QString::number(row + 1));
         }
      }
      else if (column == 1)
      {
         // Display is 1-based; storage is 0-based.
         return QVariant(QString::number(mGroupedBands[row].first.getOriginalNumber() + 1));
      }
      else if (column == 2)
      {
         // Display is 1-based; storage is 0-based.
         return QVariant(QString::number(mGroupedBands[row].second.getOriginalNumber() + 1));
      }
      else
      {
         return QVariant();
      }
   }
   else if (role == Qt::FontRole && column == 0)
   {
      // Display the output band number in bold.
      QFont font;
      font.setBold(true);
      return QVariant(font);
   }

   return QVariant();
}

void BandBinningModel::moveUp(int row)
{
   if (row > 0 && row < rowCount())
   {
      // Move the selected row up one position by swapping it with the previous element and emitting dataChanged.
      std::swap(mGroupedBands[row], mGroupedBands[row - 1]);
      QModelIndex topLeft = createIndex(row - 1, 0);
      QModelIndex bottomRight = createIndex(row, columnCount());
      emit dataChanged(topLeft, bottomRight);
   }
}

void BandBinningModel::moveDown(int row)
{
   if (row >= 0 && row + 1 < rowCount())
   {
      // Move the selected row down one position by swapping it with the next element and emitting dataChanged.
      std::swap(mGroupedBands[row], mGroupedBands[row + 1]);
      QModelIndex topLeft = createIndex(row, 0);
      QModelIndex bottomRight = createIndex(row + 1, columnCount());
      emit dataChanged(topLeft, bottomRight);
   }
}

void BandBinningModel::addGroupedBand(int row, std::pair<DimensionDescriptor, DimensionDescriptor> groupedBand)
{
   // This insertion is computationally expensive, so try to avoid using it too frequently.
   // For large changes, use setGroupedBands().
   if (row >= 0)
   {
      beginInsertRows(QModelIndex(), row, row);
      mGroupedBands.insert(mGroupedBands.begin() + row, groupedBand);
      endInsertRows();
   }
}

void BandBinningModel::setGroupedBands(
   const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands)
{
   if (groupedBands != mGroupedBands)
   {
      mGroupedBands = groupedBands;
      reset();
   }
}

const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& BandBinningModel::getGroupedBands() const
{
   return mGroupedBands;
}
