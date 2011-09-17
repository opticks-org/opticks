/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MENUANDTOOLBARTEST_H
#define MENUANDTOOLBARTEST_H

#include "ViewerShell.h"
#include "MenuAndToolBarTestGui.h"
#include "Service.h"

class MenuAndToolBarTest : public QObject, public ViewerShell
{
   Q_OBJECT

public:
    MenuAndToolBarTest();
    ~MenuAndToolBarTest();

    bool execute(PlugInArgList*, PlugInArgList*);
    QWidget* getWidget() const;

public slots:
   void dialogClosed();

private:
   MenuAndToolBarTestGui* mpGui;
};

#endif
