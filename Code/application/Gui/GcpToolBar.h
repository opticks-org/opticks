/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPTOOLBAR_H
#define GCPTOOLBAR_H

#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "ToolBarAdapter.h"

class ColorMenu;
class GcpLayer;
class GcpSymbolButton;
class Layer;

class GcpToolBar : public ToolBarAdapter
{
   Q_OBJECT

public:
   GcpToolBar(const std::string& id, QWidget* parent = 0);
   ~GcpToolBar();

   Layer* getGcpLayer() const;

public slots:
   void setEnabled(bool bEnable);
   bool setGcpLayer(Layer* pLayer);

protected:
   void gcpLayerDeleted(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void initializeColorMenu();
   void setMarkerSymbol(GcpSymbol markerSymbol);
   void setMarkerColor(const QColor& markerColor);

private:
   GcpToolBar(const GcpToolBar& rhs);
   GcpToolBar& operator=(const GcpToolBar& rhs);
   GcpSymbolButton* mpSymbol;
   QAction* mpColorAction;
   ColorMenu* mpColorMenu;

   GcpLayer* mpGcpLayer;
};

#endif
