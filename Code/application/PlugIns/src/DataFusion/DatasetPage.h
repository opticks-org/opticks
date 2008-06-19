/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATASET_PAGE_H
#define DATASET_PAGE_H

#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "FusionPage.h"
#include "Observer.h"
#include "PlugInManagerServices.h"

#include <QtCore/QMap>
#include <QtGui/QWidget>

class QCheckBox;
class QGroupBox;
class QListWidget;
class QListWidgetItem;
class SpatialDataView;
class SpatialDataWindow;

namespace boost
{
   class any;
}

class DatasetPage : public FusionPage, public Observer
{
   Q_OBJECT

public:
   DatasetPage(QWidget* pParent);
   ~DatasetPage();

   SETTING(BypassTiePointStep, DataFusion, bool, false);

   bool canBypassTiePointStep() const;

   void setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary);
   SpatialDataView* getPrimaryView() const;
   SpatialDataView* getSecondaryView() const;

   void windowAdded(Subject& subject, const std::string& signal, const boost::any& value);
   void windowDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void windowModified(Subject &subject, const std::string &signal, const boost::any &v);
   void windowAttached(Subject &subject, const std::string &signal, const boost::any &v);
   void attached(Subject &subject, const std::string &signal, const Slot &slot);
   void detached(Subject &subject, const std::string &signal, const Slot &slot);

   bool isValid() const;

protected slots:
   void hideEvent(QHideEvent* pEvt);
   void importData();

private:
   // sets the current spatial data view and the appropriate ListBox
   // this method depends on the fact that a SpatialDataView and SpatialDataWindow have the same name
   void setView(SpatialDataView* pView, QListWidget& box);

   // gets the 'current' spatial data view of a list box
   SpatialDataView* getView(QListWidget* pBox) const;

   // map from windows to index in the ListBoxes
   QMap<SpatialDataWindow*, QListWidgetItem*> mPrimaryMap, mSecondaryMap;

   QListWidget* mpSecondaryList;
   QListWidget* mpPrimaryList;
   QCheckBox* mpBypassCheckBox;
   QCheckBox* mpSaveCheckBox;
   QGroupBox* mpGroupBox;

   Service<DesktopServices> mpDesktop;
   Service<PlugInManagerServices> mpPlugMgr;

   static const std::string BYPASS_TIE_POINT_STATE;
};

#endif
