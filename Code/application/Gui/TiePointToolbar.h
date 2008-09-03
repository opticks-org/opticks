/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINT_TOOLBAR_H
#define TIEPOINT_TOOLBAR_H

#include <QtGui/QAction>

#include "ToolBarAdapter.h"

class ColorMenu;
class Layer;
class TiePointLayer;

class TiePointToolBar : public ToolBarAdapter
{
   Q_OBJECT

public:
   TiePointToolBar(const std::string& id, QWidget* parent = 0);
   ~TiePointToolBar();

   Layer* getTiePointLayer() const;

public slots:
   void setEnabled(bool bEnable);
   bool setTiePointLayer(Layer* pLayer);

protected:
   void tiePointLayerDeleted(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void updateLabelsCheckBox();

   void setLabelsOnOff();
   void initializeColorMenu();
   void setMarkerColor(const QColor& markerColor);

private:
   QAction* mpLabelsEnabledAction;
   QAction* mpColorAction;
   ColorMenu* mpColorMenu;

   TiePointLayer* mpTiePointLayer;
};

#endif
