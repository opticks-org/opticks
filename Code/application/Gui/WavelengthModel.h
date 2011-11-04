/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHMODEL_H
#define WAVELENGTHMODEL_H

#include "DimensionDescriptor.h"

#include <QtCore/QAbstractTableModel>

#include <vector>

class Wavelengths;

class WavelengthModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   WavelengthModel(QObject* pParent = NULL);
   WavelengthModel(Wavelengths* pWavelengths, QObject* pParent = NULL);
   ~WavelengthModel();

   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   virtual Qt::ItemFlags flags(const QModelIndex& index) const;
   virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
   virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

   void setWavelengths(const std::vector<DimensionDescriptor>& bands, Wavelengths* pWavelengths);
   void updateActiveWavelengths(const std::vector<DimensionDescriptor>& bands);
   void updateData();

private:
   WavelengthModel(const WavelengthModel& rhs);
   WavelengthModel& operator=(const WavelengthModel& rhs);
   Wavelengths* mpWavelengths;
   std::vector<unsigned int> mAllBands;
   std::vector<unsigned int> mActiveBands;
};

#endif
