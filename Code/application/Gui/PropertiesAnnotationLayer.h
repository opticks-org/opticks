/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESANNOTATIONLAYER_H
#define PROPERTIESANNOTATIONLAYER_H

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QWidget>

#include "Modifier.h"

#include <map>

class AnnotationLayer;
class GraphicObject;
class PropertiesGraphicObject;
class SessionItem;

class PropertiesAnnotationLayer : public QWidget
{
   Q_OBJECT

public:
   PropertiesAnnotationLayer();
   ~PropertiesAnnotationLayer();

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
   AnnotationLayer* mpAnnotationLayer;
   std::map<QListWidgetItem*, GraphicObject*> mObjects;
   Modifier mPropertiesModifier;

   QListWidget* mpObjectsList;
   QStackedWidget* mpStack;
   QLabel* mpNoObjectLabel;
   PropertiesGraphicObject* mpPropertiesWidget;
   QCheckBox* mpLockCheck;
};

#endif
