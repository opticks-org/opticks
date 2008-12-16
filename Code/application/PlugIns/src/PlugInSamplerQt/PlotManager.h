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

#include <QtCore/QObject>
#include <QtGui/QAction>

#include "AlgorithmShell.h"
#include "DesktopServices.h"

#define PLOT_MANAGER_NAME "Plot Manager"

namespace boost
{
   class any;
}

class PlotManager : public QObject, public AlgorithmShell
{
   Q_OBJECT

public:
   PlotManager();
   ~PlotManager();

   void windowHidden(Subject& subject, const std::string &signal, const boost::any& v);
   void windowShown(Subject& subject, const std::string &signal, const boost::any& v);

   bool setBatch();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

protected slots:
   void displayPlotManager(bool bDisplay);

private:
   Service<DesktopServices> mpDesktop;
   QAction* mpWindowAction;
};

#endif
