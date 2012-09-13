/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTPAGE_H
#define TIEPOINTPAGE_H

#include <QtCore/QMap>
#include <QtGui/QWidget>

#include "EnumWrapper.h"
#include "FusionPage.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "Observer.h"

class QLabel;
class QPushButton;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;
class TiePointList;

class TiePointPage : public FusionPage, public Observer
{
   Q_OBJECT

public:
   TiePointPage(QWidget* pParent);
   ~TiePointPage();

   void gcpLayerAttached(Subject& subject, const std::string& signal, const boost::any& v);
   void gcpLayerDetached(Subject& subject, const std::string& signal, const boost::any& v);
   void gcpLayerDeleted(Subject& subject, const std::string& signal, const boost::any& v);
   void gcpListDeleted(Subject& subject, const std::string& signal, const boost::any& v);
   void gcpListModified(Subject& subject, const std::string& signal, const boost::any& v);
   void gcpListDetached(Subject& subject, const std::string& signal, const boost::any& v);
   void tiePointListDeleted(Subject& subject, const std::string& signal, const boost::any& v);
   void attached(Subject& subject, const std::string& signal, const Slot& slot);
   void detached(Subject& subject, const std::string& signal, const Slot& slot);

   bool isValid() const;

   void setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary);

   const TiePointList* getTiePoints() const;

   std::string getPreferredPrimaryMouseMode() const;
   std::string getPreferredSecondaryMouseMode() const;
   Layer* getPreferredPrimaryActiveLayer() const;
   Layer* getPreferredSecondaryActiveLayer() const;

protected:
   void showEvent(QShowEvent* pEvt);
   void hideEvent(QHideEvent* pEvt);

   // creates a Fusion GcpLayer if necessary
   GcpLayer* createFusionGcpLayer(SpatialDataView* pView);

   // sets the member layer according to pLayer and performs appropriate attach/detach
   void setGcpLayer(GcpLayer*& pMemberLayer, GcpLayer* pLayer);

   // clears the ListView and adds all GCPs in pList to the ListView
   void populateListBox(GcpList& gcpList, QTreeWidget& listBox, QMap<QTreeWidgetItem*, const GcpPoint*>& map);

   // removes the current GCP from a view and the GCP List
   void removeGcp(QTreeWidget& view, const GcpLayer& layer, QMap<QTreeWidgetItem*, const GcpPoint*>& map,
      QPushButton& button);

protected slots:
   void deriveTiePoint();
   void enableGcpActions();
   void enableTiePointActions();
   void removeTiePoint();
   void removeGcp();
   void verifyTiePoint(QTreeWidgetItem* pItem, int column);

private:
   TiePointPage(const TiePointPage& rhs);
   TiePointPage& operator=(const TiePointPage& rhs);
   static const std::string FUSION_GCP_NAME;
   // the columns in the GCP CustomListViews
   enum GcpItemTypeEnum { NAME = 0, PIXEL_X = 1, PIXEL_Y = 2 };

   /**
    * @EnumWrapper TiePointPage::GcpItemTypeEnum.
    */
   typedef EnumWrapper<GcpItemTypeEnum> GcpItemType;

   // the columns in the TiePoint QListView
   enum TiePointItemTypeEnum { PRI_X = 0, PRI_Y = 1, SEC_X = 2, SEC_Y = 3 };

   /**
    * @EnumWrapper TiePointPage::TiePointItemTypeEnum.
    */
   typedef EnumWrapper<TiePointItemTypeEnum> TiePointItemType;

   const std::string TIE_POINT_NAME;

   TiePointList* mpTiePoints;

   GcpLayer* mpPrimaryGcpLayer;
   GcpLayer* mpSecondaryGcpLayer;

   QPushButton* mpDeriveButton;
   QTreeWidget* mpTiePointView;
   QSpinBox* mpZoomBox;
   QPushButton* mpSecGcpRemoveButton;
   QPushButton* mpPrimGcpRemoveButton;
   QTreeWidget* mpPrimaryGcps;
   QTreeWidget* mpSecondaryGcps;
   QPushButton* mpTiePointRemoveButton;

   QMap<QTreeWidgetItem*, const GcpPoint*> mPrimaryMap;
   QMap<QTreeWidgetItem*, const GcpPoint*> mSecondaryMap;
};

#endif
