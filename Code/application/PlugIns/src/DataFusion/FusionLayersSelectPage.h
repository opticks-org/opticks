/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FUSIONLAYERSSELECTPAGE_H
#define FUSIONLAYERSSELECTPAGE_H

#include <QtCore/QMap>
#include <QtGui/QCheckBox>

#include "FusionPage.h"
#include "Observer.h"

#include <vector>

class QTreeWidget;
class QTreeWidgetItem;
class Layer;
class SpatialDataView;

class FusionLayersSelectPage : public FusionPage, public Observer
{
   Q_OBJECT
public:
   FusionLayersSelectPage(QWidget* pParent);
   ~FusionLayersSelectPage();

   void setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary);

   bool areAllSelectedLayersAvailable() const;

   std::vector<Layer*> getSelectedLayers() const;

   bool isValid() const;

   void layerDeleted(Subject& subject, const std::string& signal, const boost::any& v);
   void layerModified(Subject& subject, const std::string& signal, const boost::any& v);
   void layerListDeleted(Subject& subject, const std::string& signal, const boost::any& v);
   void layerListModified(Subject& subject, const std::string& signal, const boost::any& v);
   void layerListAttached(Subject& subject, const std::string& signal, const boost::any& v);
   void attached(Subject& subject, const std::string& signal, const Slot& slot);
   void detached(Subject& subject, const std::string& signal, const Slot& slot);

protected:
   void hideEvent(QHideEvent* pEvt);

private:
   FusionLayersSelectPage(const FusionLayersSelectPage& rhs);
   FusionLayersSelectPage& operator=(const FusionLayersSelectPage& rhs);
   void addLayerToGui(Layer* pLayer);

   const static QString LAYER_NAME_COLUMN;
   const static QString LAYER_TYPE_COLUMN;

   std::vector<Layer*> mSelectedLayers;

   typedef QMap<Layer*, QTreeWidgetItem*> LayerMap;

   QTreeWidget* mpLayerView;
   LayerMap mLayers;
};

#endif
