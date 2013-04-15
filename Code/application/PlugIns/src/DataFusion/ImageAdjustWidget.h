/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEADJUSTWIDGET_H
#define IMAGEADJUSTWIDGET_H

#include "AttachmentPtr.h"
#include "DesktopServices.h"
#include "Layer.h"
#include "LayerList.h"
#include "SpatialDataView.h"

#include <boost/any.hpp>
#include <QtCore/QMetaType>
#include <QtGui/QWidget>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTimer;
class QwtKnob;
class QwtSlider;
class QwtWheel;

Q_DECLARE_METATYPE(Layer*)

class ImageAdjustWidget : public QWidget
{
   Q_OBJECT

public:
   ImageAdjustWidget(WorkspaceWindow* pWindow, QWidget* pParent);
   virtual ~ImageAdjustWidget();

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   void windowActivated(Subject& subject, const std::string& signal, const boost::any& value);
   void layerListChanged(Subject& subject, const std::string& signal, const boost::any& value);
   void layerVisibilityChanged(Subject& subject, const std::string& signal, const boost::any& value);
   void layerPropertyChanged(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void updateOffset(double value);
   void updateControls();
   void updateAlpha(double value);
   void changeFlickerRate(double rate);
   void changeVisibility(bool visible);
   void changeVisibility();
   void toggleAutoFlicker(bool state);
   void changeLayerOffset(double value);
   void changeLayerScale(double value);

private:
   ImageAdjustWidget(const ImageAdjustWidget& rhs);
   ImageAdjustWidget& operator=(const ImageAdjustWidget& rhs);
   void resetLayers();
   Layer* getLayer() const;

   AttachmentPtr<DesktopServices> mpDesktop;
   double mXOffsetPrev;
   double mYOffsetPrev;
   AttachmentPtr<SpatialDataView> mpView;
   AttachmentPtr<LayerList> mpLayerList;
   AttachmentPtr<Layer> mpLayerAttachment;
   QTimer* mpTimer;
   QComboBox* mpLayerSelection;
   QwtWheel* mpXOffsetWheel;
   QDoubleSpinBox* mpXOffset;
   QwtWheel* mpYOffsetWheel;
   QDoubleSpinBox* mpYOffset;
   QDoubleSpinBox* mpXScale;
   QDoubleSpinBox* mpYScale;
   QwtKnob* mpFramerateKnob;
   QDoubleSpinBox* mpFlickerRate;
   QPushButton* mpAutoFlickerButton;
   QPushButton* mpManualFlickerButton;
   QLabel* mpTransparencyLabel;
   QwtSlider* mpAlpha;
};

#endif
