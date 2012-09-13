/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLOTMANAGER_H
#define PLOTMANAGER_H

#include "DockWindowShell.h"

#define PLOT_MANAGER_NAME "Plot Manager"

class PlotManager : public DockWindowShell
{
public:
   PlotManager();
   ~PlotManager();

protected:
   QAction* createAction();
   QWidget* createWidget();

private:
   PlotManager(const PlotManager& rhs);
   PlotManager& operator=(const PlotManager& rhs);
};

#endif
