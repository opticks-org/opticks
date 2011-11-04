/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURECLASSWIDGET_H
#define FEATURECLASSWIDGET_H

#include <QtGui/QWidget>

#include "ConnectionParameters.h"
#include "QueryOptions.h"

namespace ArcProxyLib
{
   class FeatureClassProperties;
}
class ConnectionParametersWidget;
class FeatureClass;
class LatLonLineEdit;
class ListInspectorWidget;

class QLabel;
class QLineEdit;
class QListWidgetItem;
class QProgressBar;
class QRadioButton;

#include <map>

class FeatureClassWidget : public QWidget
{
   Q_OBJECT

public:
   FeatureClassWidget(QWidget* pParent = NULL);
   ~FeatureClassWidget();

   bool applyChanges();
   void initialize(FeatureClass *pFeatureClass);

   void setAvailableConnectionTypes(const std::vector<ArcProxyLib::ConnectionType> &types);

public slots:
   void testConnection(bool onlyIfModified = true);
   void addDisplayItems();
   void saveDisplayInspector(QWidget *pInspector, QListWidgetItem *pItem);
   void loadDisplayInspector(QWidget *pInspector, QListWidgetItem *pItem);
   void removeDisplayItem(QListWidgetItem *pItem);
   void clipButtonClicked();
   
protected:
   void setFeatureClassProperties(const ArcProxyLib::FeatureClassProperties &featureClassProperties,
      ArcProxyLib::ConnectionType connectionType);

private:
   FeatureClassWidget(const FeatureClassWidget& rhs);
   FeatureClassWidget& operator=(const FeatureClassWidget& rhs);
   ConnectionParametersWidget* mpConnection;
   ListInspectorWidget* mpDisplay;
   QWidget* mpClipping;

   QLineEdit* mpLayerNameEdit;

   // clipping
   QRadioButton* mpNoClipButton;
   QRadioButton* mpSceneClipButton;
   QRadioButton* mpSpecifiedClipButton;
   LatLonLineEdit* mpNorthEdit;
   LatLonLineEdit* mpSouthEdit;
   LatLonLineEdit* mpEastEdit;
   LatLonLineEdit* mpWestEdit;

   FeatureClass* mpFeatureClass;
   std::map<QListWidgetItem*, QueryOptions> mQueries;

   QLabel* mpErrorLabel;
   QProgressBar* mpProgressBar;

   std::vector<QWidget*> mSpecifiedClipWidgets;
};

#endif
