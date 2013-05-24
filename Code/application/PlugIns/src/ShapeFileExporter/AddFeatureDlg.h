/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ADDFEATUREDLG_H
#define ADDFEATUREDLG_H

#include "GraphicElement.h"
#include "GraphicObject.h"
#include "ShapeFileTypes.h"

#include <QtCore/QMetaType>
#include <QtGui/QDialog>

#include <map>
#include <vector>

class QCheckBox;
class QComboBox;
class QTreeWidget;

Q_DECLARE_METATYPE(GraphicElement*)
Q_DECLARE_METATYPE(GraphicObject*)

class AddFeatureDlg : public QDialog
{
   Q_OBJECT

public:
   AddFeatureDlg(const std::vector<GraphicElement*>& graphicElements,
                 ShapefileTypes::ShapeType shapeType, QWidget* pParent = NULL);
   virtual ~AddFeatureDlg();

   std::map<GraphicElement*, std::vector<GraphicObject*> > getFeatureItems() const;
   QString getFeatureClass() const;

public slots:
   virtual void accept();

private:
   AddFeatureDlg(const AddFeatureDlg& rhs);
   AddFeatureDlg& operator=(const AddFeatureDlg& rhs);

   QTreeWidget* mpElementTree;
   QCheckBox* mpFeatureClassCheck;
   QComboBox* mpFeatureClassCombo;
};

#endif
