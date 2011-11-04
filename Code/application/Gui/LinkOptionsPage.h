/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LINKOPTIONSPAGE_H
#define LINKOPTIONSPAGE_H

#include "TypesFile.h"

#include <QtCore/QMap>
#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QWidget>

#include <vector>

class CustomTreeWidget;
class Layer;
class View;

class LinkOptionsPage : public QWidget
{
   Q_OBJECT

public:
   LinkOptionsPage(QWidget* parent = 0);
   ~LinkOptionsPage();

   // Views
   std::vector<View*> getViewLinks() const;
   LinkType getLinkType() const;
   bool isTwoWayLink() const;

   // Layers
   std::vector<Layer*> getLayerLinks() const;
   bool areLayersDuplicated() const;

public slots:
   void setLinkObjects(const QString& strDataset);

protected slots:
   void activateOptions();

private:
   LinkOptionsPage(const LinkOptionsPage& rhs);
   LinkOptionsPage& operator=(const LinkOptionsPage& rhs);
   CustomTreeWidget* mpLinksTree;
   QMap<QTreeWidgetItem*, View*> mViews;
   QMap<QTreeWidgetItem*, Layer*> mLayers;

   QString mstrDataset;

   QStackedWidget* mpStack;
   QWidget* mpViewWidget;
   QRadioButton* mpAutolinkRadio;
   QRadioButton* mpMirrorRadio;
   QRadioButton* mpGeoRadio;
   QRadioButton* mpUnlinkRadio;
   QCheckBox* mpTwoWayCheck;
   QWidget* mpLayerWidget;
   QCheckBox* mpDuplicateCheck;
};

#endif
