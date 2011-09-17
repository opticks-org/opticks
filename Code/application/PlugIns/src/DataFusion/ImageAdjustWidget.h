/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEADJUSTWIDGET_H
#define IMAGEADJUSTWIDGET_H

#include "AttachmentPtr.h"
#include "DesktopServices.h"
#include "LayerList.h"

#include <boost/any.hpp>
#include <QtGui/QWidget>

class ElidedLabel;
class QGroupBox;
class QLabel;
class QPushButton;
class QSlider;
class QTimer;
class RasterLayer;
class SpatialDataView;
class SpatialDataWindow;
class WorkspaceWindow;

class ImageAdjustWidget : public QWidget
{
   Q_OBJECT

public:
   ImageAdjustWidget(WorkspaceWindow* pWindow, QWidget* pParent);
   ~ImageAdjustWidget();

   void windowActivated(Subject& subject, const std::string& signal, const boost::any& value);
   void layerListChanged(Subject& subject, const std::string& signal, const boost::any& value);

protected:
   void resetWidgets();

protected slots:
   void runFlicker(); // actually performs flickering
   void startFlicker(); // starts timer, etc.

   double computeFlickerRate(int position) const;
   void changeFlickerRate(int position);
   void changeFlickerAlpha(int position);

   void stopFlicker();

   void shift(int value);

private:
   QTimer* mpTimer;
   QGroupBox* mpAutomatedFlickerBox;
   QGroupBox* mpManualFlickerBox;
   QLabel* mpFlickerMax;
   ElidedLabel* mpCurrentCubeLayerLabel;
   ElidedLabel* mpCurrentLayerLabel;
   QLabel* mpCurrentFlickerSpeed;
   QSlider* mpFlickerSlider;
   QSlider* mpOpacitySlider;
   QSlider* mpFlickerYSlider;
   QSlider* mpFlickerXSlider;
   QPushButton* mpManualFlicker;

   RasterLayer* mpPrimaryLayer;
   RasterLayer* mpSecondaryLayer;
   AttachmentPtr<LayerList> mpLayerList;
   AttachmentPtr<DesktopServices> mpDesktopServices;

   void setLayerList(WorkspaceWindow* pWindow);
   std::string getLayerName(RasterLayer* pRasterLayer);
   DataElement* getDataElement(RasterLayer* pRasterLayer);
   void resetLayers();
   void resetSliders();
   void resetLabels(std::string layerName, std::string cubeLayerName);
};

#endif
