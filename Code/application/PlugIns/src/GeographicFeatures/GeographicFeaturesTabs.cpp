/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <map>
#include <string>

#include <QtGui/QTabWidget>

#include "FeatureTable.h"
#include "GeographicFeaturesTabs.h"
#include "GraphicLayer.h"

GeographicFeaturesTabs::GeographicFeaturesTabs(QWidget* pParent) : QTabWidget(pParent)
{
   setTabPosition(QTabWidget::South);
}

GeographicFeaturesTabs::~GeographicFeaturesTabs()
{}

bool GeographicFeaturesTabs::addTab(FeatureClass* pFeatureClass, GraphicLayer* pLayer)
{
   bool success = false;

   if ((pFeatureClass != NULL) && (pLayer != NULL))
   {
      FeatureTable* pTable = new FeatureTable(pFeatureClass, pLayer);

      VERIFYNR(pLayer->attach(SIGNAL_NAME(Layer, NameChanged),
         Slot(this, &GeographicFeaturesTabs::renameFeatureTableTab)));

      int tabIndex = QTabWidget::addTab(pTable, QString::fromStdString(pLayer->getName()));
      setCurrentIndex(tabIndex);
      mTables.insert(std::pair<Layer*, FeatureTable*>(pLayer, pTable));

      success = true;
   }

   return success;
}

bool GeographicFeaturesTabs::removeTab(Layer* pLayer)
{
   std::map<Layer*, FeatureTable*>::iterator tableIter = mTables.find(pLayer);

   if (tableIter != mTables.end())
   {
      int index = QTabWidget::indexOf(tableIter->second);
      QTabWidget::removeTab(index);
      mTables.erase(tableIter);
   }
   return true;
}

int GeographicFeaturesTabs::getTabId(Layer* pLayer) const
{
   int tabId = -1;
   std::map<Layer*, FeatureTable*>::const_iterator iter = mTables.find(pLayer);
   if (mTables.end() != iter)
   {
      tabId = QTabWidget::indexOf(iter->second);
   }
   return tabId;
}

bool GeographicFeaturesTabs::activateTab(Layer* pLayer)
{
   bool success = false;
   std::map<Layer*, FeatureTable*>::const_iterator iter = mTables.find(pLayer);
   if (mTables.end() != iter)
   {
      setCurrentWidget(iter->second);
      success = true;
   }

   return success;
}

void GeographicFeaturesTabs::renameFeatureTableTab(Subject& subject, const std::string& signal, const boost::any& data)
{
   GraphicLayer* pGraphicLayer = dynamic_cast<GraphicLayer*>(&subject);
   if (pGraphicLayer != NULL)
   {
      std::string newName = boost::any_cast<std::string>(data);
      int tabIndex = getTabId(pGraphicLayer);
      if (tabIndex >= 0 && tabIndex < count())
      {
         setTabText(tabIndex, QString::fromStdString(newName));
      }
   }
}
