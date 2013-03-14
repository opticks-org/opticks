/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BANDBINNINGMODEL_H
#define BANDBINNINGMODEL_H

#include <QtCore/QAbstractTableModel>

#include "DimensionDescriptor.h"

#include <utility>
#include <vector>

class RasterDataDescriptor;

class BandBinningModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   BandBinningModel(const RasterDataDescriptor* pDescriptor, QObject* pParent = NULL);
   virtual ~BandBinningModel();

   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   virtual Qt::ItemFlags flags(const QModelIndex& index) const;
   virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
   virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

   void moveUp(int row);
   void moveDown(int row);

   void addGroupedBand(int row, std::pair<DimensionDescriptor, DimensionDescriptor> groupedBand);
   void setGroupedBands(const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands);
   const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& getGroupedBands() const;

private:
   BandBinningModel(const BandBinningModel& rhs);
   BandBinningModel& operator=(const BandBinningModel& rhs);
   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > mGroupedBands;
   const RasterDataDescriptor* mpDescriptor;
};

#endif
