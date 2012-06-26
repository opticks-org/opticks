/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DISPLAYTOOLBAR_H
#define DISPLAYTOOLBAR_H

#include <QtGui/QComboBox>

#include "ToolBarAdapter.h"

class DisplayToolBar : public ToolBarAdapter
{
   Q_OBJECT

public:
   DisplayToolBar(const std::string& id, QWidget* parent = 0);
   ~DisplayToolBar();

   void addPercentageCombo();

public slots:
   void enablePercentageCombo(bool bEnable);
   void setZoomPercentage(double dPercent);

signals:
   void zoomChanged(double dPercent);

protected slots:
   void zoomToComboValue(int iIndex);
   void zoomToCustomValue();

private:
   DisplayToolBar(const DisplayToolBar& rhs);
   DisplayToolBar& operator=(const DisplayToolBar& rhs);
   QComboBox* mpPercentCombo;
};

#endif
