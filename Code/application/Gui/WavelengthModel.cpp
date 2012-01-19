/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QFont>

#include "Wavelengths.h"
#include "WavelengthModel.h"

#include <algorithm>
#include <vector>

WavelengthModel::WavelengthModel(QObject* pParent) :
   QAbstractTableModel(pParent),
   mpWavelengths(NULL)
{}

WavelengthModel::WavelengthModel(Wavelengths* pWavelengths, QObject* pParent) :
   QAbstractTableModel(pParent),
   mpWavelengths(pWavelengths)
{}

WavelengthModel::~WavelengthModel()
{}

QVariant WavelengthModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
   {
      if (section == 0)
      {
         return QVariant("Band");
      }
      else if (section == 1)
      {
         return QVariant("Start");
      }
      else if (section == 2)
      {
         return QVariant("Center");
      }
      else if (section == 3)
      {
         return QVariant("End");
      }
   }

   return QVariant();
}

int WavelengthModel::rowCount(const QModelIndex& parent) const
{
   return static_cast<int>(mAllBands.size());
}

int WavelengthModel::columnCount(const QModelIndex& parent) const
{
   return 4;
}

Qt::ItemFlags WavelengthModel::flags(const QModelIndex& index) const
{
   Qt::ItemFlags flag = QAbstractTableModel::flags(index);
   if (index.isValid())
   {
      int column = index.column();
      if (column == 1 || column == 2 || column == 3)
      {
         flag |= Qt::ItemIsEditable;
      }
   }

   return flag;
}

bool WavelengthModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
   if (index.isValid() == false)
   {
      return false;
   }

   if ((role == Qt::EditRole) && (mpWavelengths != NULL))
   {
      int row = index.row();
      int column = index.column();

      // Get the wavelength vector containing the value that was edited and the value units
      std::vector<double> wavelengthValues;

      if (column == 1)
      {
         wavelengthValues = mpWavelengths->getStartValues();
      }
      else if (column == 2)
      {
         wavelengthValues = mpWavelengths->getCenterValues();
      }
      else if (column == 3)
      {
         wavelengthValues = mpWavelengths->getEndValues();
      }

      WavelengthUnitsType units = mpWavelengths->getUnits();

      // Get the edited wavelength value
      double wavelength = 0.0;
      bool validWavelengths = false;
      bool multipleValuesChanged = false;

      QString valueText = value.toString();
      if (valueText.isEmpty() == true)
      {
         for (int i = 0; i < static_cast<int>(wavelengthValues.size()); ++i)
         {
            if ((wavelengthValues[i] > 0.0) && (i != row))
            {
               validWavelengths = true;
               break;
            }
         }

         if ((validWavelengths == false) && (wavelengthValues.empty() == false))
         {
            wavelengthValues.clear();
            multipleValuesChanged = true;
         }
      }
      else
      {
         bool success = false;
         wavelength = valueText.toDouble(&success);
         if (success == false)
         {
            return false;
         }

         validWavelengths = true;
      }

      // Update the wavelength value in the vector
      if ((wavelengthValues.empty() == true) && (validWavelengths == true))
      {
         wavelengthValues.resize(mAllBands.size(), 0.0);
         multipleValuesChanged = true;
      }

      if ((row > -1) && (row < static_cast<int>(wavelengthValues.size())))
      {
         wavelengthValues[row] = wavelength;
      }

      // Update the vector in the wavelengths
      if (column == 1)
      {
         mpWavelengths->setStartValues(wavelengthValues, units);
      }
      else if (column == 2)
      {
         mpWavelengths->setCenterValues(wavelengthValues, units);
      }
      else if (column == 3)
      {
         mpWavelengths->setEndValues(wavelengthValues, units);
      }

      if (multipleValuesChanged == true)
      {
         QModelIndex topLeft = createIndex(0, column);
         QModelIndex bottomRight = createIndex(static_cast<int>(mAllBands.size() - 1), column);
         emit dataChanged(topLeft, bottomRight);
      }
      else
      {
         emit dataChanged(index, index);
      }

      return true;
   }

   return false;
}

QVariant WavelengthModel::data(const QModelIndex& index, int role) const
{
   if (index.isValid() == false)
   {
      return QVariant();
   }

   unsigned int row = static_cast<unsigned int>(index.row());
   int column = index.column();

   if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
   {
      if (column == 0)
      {
         if (row < mAllBands.size())
         {
            return QVariant(QString::number(mAllBands[row] + 1));
         }
      }
      else if (mpWavelengths != NULL)
      {
         double wavelength = 0.0;
         if (column == 1)
         {
            const std::vector<double>& startWavelengths = mpWavelengths->getStartValues();
            if (row < startWavelengths.size())
            {
               wavelength = startWavelengths[row];
            }
         }
         else if (column == 2)
         {
            const std::vector<double>& centerWavelengths = mpWavelengths->getCenterValues();
            if (row < centerWavelengths.size())
            {
               wavelength = centerWavelengths[row];
            }
         }
         else if (column == 3)
         {
            const std::vector<double>& endWavelengths = mpWavelengths->getEndValues();
            if (row < endWavelengths.size())
            {
               wavelength = endWavelengths[row];
            }
         }
         else
         {
            return QVariant();
         }

         if (wavelength > 0.0)
         {
            return QVariant(QString::number(wavelength));
         }
      }
   }
   else if (role == Qt::FontRole)
   {
      QFont font = QApplication::font();
      if (row < mAllBands.size())
      {
         font.setBold(std::binary_search(mActiveBands.begin(), mActiveBands.end(), mAllBands[row]));
      }

      return QVariant(font);
   }

   return QVariant();
}

void WavelengthModel::setWavelengths(const std::vector<DimensionDescriptor>& bands, Wavelengths* pWavelengths)
{
   if (pWavelengths != mpWavelengths)
   {
      // Update the wavelengths
      mpWavelengths = pWavelengths;

      // Update the band numbers
      mAllBands.resize(bands.size());
      for (std::vector<DimensionDescriptor>::size_type i = 0; i < bands.size(); ++i)
      {
         mAllBands[i] = bands[i].getOriginalNumber();
      }

      mActiveBands.clear();

      // Reset the internal state of the model
      reset();
   }
}

void WavelengthModel::updateActiveWavelengths(const std::vector<DimensionDescriptor>& bands)
{
   if (mAllBands.empty() == true)
   {
      return;
   }

   mActiveBands.resize(bands.size());
   for (std::vector<DimensionDescriptor>::size_type i = 0; i < bands.size(); ++i)
   {
      mActiveBands[i] = bands[i].getOriginalNumber();
   }

   std::sort(mActiveBands.begin(), mActiveBands.end());

   QModelIndex topLeft = createIndex(0, 0);
   QModelIndex bottomRight = createIndex(static_cast<int>(mAllBands.size() - 1), 0);
   emit dataChanged(topLeft, bottomRight);
}

void WavelengthModel::updateData()
{
   reset();
}
