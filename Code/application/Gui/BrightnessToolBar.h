/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BRIGHTNESSTOOLBAR_H
#define BRIGHTNESSTOOLBAR_H

#include <QtCore/QMap>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QSlider>

#include "ToolBarAdapter.h"
#include "TypesFile.h"

class Layer;
class RasterLayer;

/**
 *  Provide a way to adjust the Contrast and Brightness of the Image.  This should always be
 *  kept insync with the Histogram Window.
 *
 *  The Brightness toolbar is divided into three parts.  The first part adjusts the 
 *  brightness of the image with values ranging from 0 to 1.  0 brightness makes the 
 *  image black, 1 brightness makes the image black.  It has an associated non-editable text box
 *  to display the current value.
 *
 *  The second part of the toolbar adjusts the contrast of the image with values ranging
 *  from 0 to 1.  0 contrast creates many shades in the image, a contrast of one two colors
 *  in the image.  It has an associated non-editable text box to display the current value.
 *
 *  The third part of the toolbar is a combo box containing the names of the tabs displayed in
 *  the Histogram window.  Changing the value of the combo box which band is currently selected in the GUI.
 */
class BrightnessToolBar : public ToolBarAdapter
{
   Q_OBJECT

public:
   /**
    *  Creates the Brightness toolbar.
    *
    *  @param    id
    *            The unique ID for the toolbar.
    *  @param    parent
    *            The widget to which the toolbar is attached.
    */
   BrightnessToolBar(const std::string& id, QWidget* parent = 0);

   /**
    *  Destroys the Brightness toolbar.
    */
   ~BrightnessToolBar();

   Layer* getCurrentLayer() const;

public slots:
   void updateForNewView(); //called by ApplicationWindow when current window is changed.
   void setCurrentLayer(Layer* pLayer, const RasterChannelType& eColor,
      bool bRgb = false); //called by HistogramWindow when the layer and band change

signals:
   void layerActivated(Layer* pLayer, const RasterChannelType& eColor);

private slots:
   void updateValues();
   void updateValues(const RasterChannelType& eColor, double dLower, double dUpper);
   void updateBrightnessLabel(int iValue);
   void updateContrastLabel(int iValue);
   void adjustLayerStretch();
   void updateLayerCombo(bool updateCurrentRasterLayer);
   void updateBandCombo(bool updateCurrentBand);
   void onBandSelectionChange(int newIndex); //called by band combo box
   void onLayerSelectionChange(int newIndex); //called by layer comb box
   void reset();
   void enableSliders();

private:
   void onRasterLayerDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   QSlider* mpBrightnessSlider;
   QLabel* mpBrightnessText;
   QSlider* mpContrastSlider;
   QLabel* mpContrastText;
   QComboBox* mpBandCombo;
   QComboBox* mpLayerCombo;
   QAction* mpResetAction;

   RasterChannelType mRasterChannelType;
   bool mRgb;

   std::vector<RasterLayer*> mLayers;
   RasterLayer* mpRasterLayer;
};

#endif
