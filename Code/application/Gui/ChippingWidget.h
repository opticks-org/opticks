/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CHIPPINGWIDGET_H
#define CHIPPINGWIDGET_H

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

#include "DimensionDescriptor.h"
#include "LocationType.h"
#include "ObjectResource.h"
#include "RasterDataDescriptor.h"

#include <vector>

class RasterLayer;
class SpatialDataView;
class ZoomPanWidget;

class ChippingWidget : public QWidget
{
   Q_OBJECT

public:
   ChippingWidget(SpatialDataView* pView, const RasterDataDescriptor* pDefaultDescriptor, QWidget* pParent);
   virtual ~ChippingWidget();

   void setExportMode(bool enableExportMode);

   const std::vector<DimensionDescriptor>& getChipRows() const;
   const std::vector<DimensionDescriptor>& getChipColumns() const;
   const std::vector<DimensionDescriptor>& getChipBands() const;

signals:
   void chipChanged();

protected:
   void showEvent(QShowEvent* pEvent);
   RasterLayer* getRasterLayer() const;
   const RasterDataDescriptor* getDataDescriptor() const;
   bool event(QEvent* pEvent);

protected slots:
   void updateChip();
   void updateCoordinateText(const LocationType& pixelCoord);
   void showAdvanced();
   void zoomExtents();
   void centerView();

private:
   bool mExportMode;
   SpatialDataView* mpView;
   const RasterDataDescriptor* mpDefaultDescriptor;
   mutable DataDescriptorResource<RasterDataDescriptor> mpDataDescriptor;
   std::vector<DimensionDescriptor> mChipRows;
   std::vector<DimensionDescriptor> mChipColumns;
   std::vector<DimensionDescriptor> mChipBands;
   unsigned int mRowSkipFactor;
   unsigned int mColumnSkipFactor;

   ZoomPanWidget* mpSelectionWidget;
   QComboBox* mpSizeCombo;
   QLabel* mpCoordLabel;

   std::vector<unsigned int> mChipSizes;
   void initializeComboBox();
};

#endif
