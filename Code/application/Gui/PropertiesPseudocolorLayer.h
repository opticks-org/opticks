/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESPSEUDOCOLORLAYER_H
#define PROPERTIESPSEUDOCOLORLAYER_H

#include <QtGui/QTreeWidgetItem>

#include "LabeledSectionGroup.h"

#include <string>

class CustomTreeWidget;
class PseudocolorLayer;
class SessionItem;
class SymbolTypeButton;

class PropertiesPseudocolorLayer : public LabeledSectionGroup
{
   Q_OBJECT

public:
   PropertiesPseudocolorLayer();
   ~PropertiesPseudocolorLayer();

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
   void updateColorBox(QTreeWidgetItem* pItem, int iColumn);
   void updateRgbText(QTreeWidgetItem* pItem, int iColumn);
   void addClass();
   void removeSelectedClasses();

private:
   PropertiesPseudocolorLayer(const PropertiesPseudocolorLayer& rhs);
   PropertiesPseudocolorLayer& operator=(const PropertiesPseudocolorLayer& rhs);
   PseudocolorLayer* mpPseudocolorLayer;

   // Pixel marker
   SymbolTypeButton* mpSymbolButton;

   // Classes
   CustomTreeWidget* mpClassesTree;
};

#endif
