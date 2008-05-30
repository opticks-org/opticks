/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef WindowsControl_H
#define WindowsControl_H

#include "DesktopServices.h"

#include "AlgorithmShell.h"
#include "Progress.h"

class DockWindow;

class WindowsControl : public AlgorithmShell
{
public:
   WindowsControl();
   ~WindowsControl();

   bool setBatch();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

private:
   bool createWidget(DockWindow *pWindow);
   Service<DesktopServices> mpDesktop;
   Progress* mpProgress;
};

#endif
