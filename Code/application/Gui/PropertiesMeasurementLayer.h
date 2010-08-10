/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESMEASUREMENTLAYER_H
#define PROPERTIESMEASUREMENTLAYER_H

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QWidget>

#include "Modifier.h"

#include <map>

class MeasurementLayer;
class GraphicObject;
class PropertiesMeasurementObject;
class SessionItem;

class PropertiesMeasurementLayer : public QWidget
{
   Q_OBJECT

public:
   PropertiesMeasurementLayer();
   ~PropertiesMeasurementLayer();

   bool initialize(SessionItem* pSessionItem);
   bool applyChanges();

   static const std::string& getName();
   static const std::string& getPropertiesName();
   static const std::string& getDescription();
   static const std::string& getShortDescription();
   static const std::string& getCreator();
   static const std::string& getCopyright();
   static const std::string& getVersion();
   static const std::string& getDescriptorId();
   static bool isProduction();

protected slots:
   void updateProperties();

private:
   MeasurementLayer* mpMeasurementLayer;
   std::map<QListWidgetItem*, GraphicObject*> mObjects;
   Modifier mPropertiesModifier;
   Modifier mNameModifier;

   QListWidget* mpObjectsList;
   QStackedWidget* mpStack;
   QLabel* mpNoObjectLabel;
   PropertiesMeasurementObject* mpPropertiesWidget;
   QCheckBox* mpLockCheck;
};

#endif
