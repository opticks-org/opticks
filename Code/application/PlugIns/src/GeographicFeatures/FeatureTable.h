/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURETABLE_H
#define FEATURETABLE_H

#include <QtGui/QTableView>

#include <boost/any.hpp>

#include "AttachmentPtr.h"
#include "GraphicLayer.h"

class QContextMenuEvent;
class QItemSelection;
class QMenu;

class FeatureClass;
class Subject;

class FeatureTable : public QTableView
{
   Q_OBJECT

public:
   FeatureTable(FeatureClass* pFeatureClass, GraphicLayer* pFeatureLayer,  // Pass feature layer, because cannot count
      QWidget* pParent = NULL);                                            // on FeatureClass being aware of the Layer,
                                                                           // when listening to LayerList::layerAdded.
   virtual ~FeatureTable();

   GraphicLayer* getFeatureLayer();
   const GraphicLayer* getFeatureLayer() const;

protected:
   int getObjectRow(const GraphicObject* pObject) const;
   void selectRows(Subject& subject, const std::string& signal, const boost::any& data);
   void selectGraphicObjects(const QItemSelection& selection, bool selected = true);

protected slots:
   void displayContextMenu(const QPoint& pos);
   void zoomToSelected();
   void exportSelected();
   void selectGraphicObjects(const QItemSelection&, const QItemSelection&);

private:
   AttachmentPtr<GraphicLayer> mpFeatureLayer;
};

#endif