/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOGRAPHICFEATURESTABS_H
#define GEOGRAPHICFEATURESTABS_H

#include <map>

#include <boost/any.hpp>

#include <QtGui/QTabWidget>

class QWidget;
class FeatureClass;
class FeatureTable;
class GraphicLayer;
class Layer;
class Subject;

class GeographicFeaturesTabs : public QTabWidget
{
   Q_OBJECT

public:
   GeographicFeaturesTabs(QWidget* pParent = NULL);
   virtual ~GeographicFeaturesTabs();
   bool addTab(FeatureClass* pFeatureClass, GraphicLayer* pLayer);
   bool removeTab(Layer* pLayer);
   bool activateTab(Layer* pLayer);

protected:
   int getTabId(Layer* pLayer) const;
   void renameFeatureTableTab(Subject& subject, const std::string& signal, const boost::any& data);

private:
   std::map<Layer*, FeatureTable*> mTables;
};

#endif
