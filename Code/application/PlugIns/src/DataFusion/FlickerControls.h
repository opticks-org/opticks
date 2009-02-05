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

#include <QtCore/QObject>
#include <QtGui/QAction>

#include "AlgorithmShell.h"
#include "DesktopServices.h"
#include "Observer.h"

class DockWindow;

namespace boost
{
   class any;
}

class FlickerControls : public QObject, public AlgorithmShell
{
   Q_OBJECT

public:
   FlickerControls();
   ~FlickerControls();

   void windowHidden(Subject& subject, const std::string& signal, const boost::any& v);
   void windowShown(Subject& subject, const std::string& signal, const boost::any& v);

   bool execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);

   bool setBatch();

   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

protected slots:
   void displayFlickerWindow(bool bDisplay);
   bool createFlickerWindow();
   void createMenuItem();
   void attachToFlickerWindow(DockWindow* pFlickerWindow);

private:
   Service<DesktopServices> mpDesktop;
   QAction* mpWindowAction;
};

#endif
