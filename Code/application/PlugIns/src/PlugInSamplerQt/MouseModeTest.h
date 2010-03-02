/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOUSEMODETEST_H
#define MOUSEMODETEST_H

#include <QtCore/QObject>

#include "ViewerShell.h"

class MouseModeTestGui;

class MouseModeTest : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   MouseModeTest();
   ~MouseModeTest();

public:
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   QWidget* getWidget() const;

protected slots:
   void dialogClosed();

private:
   MouseModeTestGui* mpGui;
};

#endif
