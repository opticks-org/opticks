/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FLICKERCONTROLS_H
#define FLICKERCONTROLS_H

#include "DockWindowShell.h"

class FlickerControls : public DockWindowShell
{
public:
   FlickerControls();
   ~FlickerControls();

protected:
   QAction* createAction();
   QWidget* createWidget();
};

#endif
