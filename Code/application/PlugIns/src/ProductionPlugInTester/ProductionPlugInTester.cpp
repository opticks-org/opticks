/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServices.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "ProductionPlugInTester.h"

#include <QtCore/QString>
#include <QtGui/QMessageBox>

#include <string>

using namespace std;

ProductionPlugInTester::ProductionPlugInTester()
{
   setName("Production Plug-In Tester");
   setMenuLocation("[Developer Tools]\\List Not For Production Plug-Ins");
   setProductionStatus(true);
   setDescriptorId("{65DE2A1D-AA77-42f3-9FE8-425017CC79F5}");
   allowMultipleInstances(false);
}

bool ProductionPlugInTester::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ProductionPlugInTester::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ProductionPlugInTester::setBatch()
{
   AlgorithmShell::setBatch();
   return false;
}

bool ProductionPlugInTester::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (isBatch())
   {
      return false;
   }
   vector<PlugInDescriptor*> allPlugins = mpPlugMgr->getPlugInDescriptors();
   QString msg = "The following are plug-ins which return false from PlugIn::isProduction()\n";
   for (vector<PlugInDescriptor*>::const_iterator it = allPlugins.begin(); it != allPlugins.end(); ++it)
   {
      PlugInDescriptor* pDescriptor = *it;
      if (pDescriptor == NULL)
      {
         continue;
      }
      if (pDescriptor->isProduction() == false)
      {
         msg += "   " + QString::fromStdString(pDescriptor->getName()) + "\n";
      }
   }

   Service<DesktopServices> pDesktop;
   QMessageBox::information(pDesktop->getMainWidget(), "Not For Production Plug-In List", msg);

   return true;
}
