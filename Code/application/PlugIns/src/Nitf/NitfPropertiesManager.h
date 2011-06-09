/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFPROPERTIESMANAGER_H__
#define NITFPROPERTIESMANAGER_H__

#include "AttachmentPtr.h"
#include "DesktopServices.h"
#include "ExecutableShell.h"
#include "PlugInResource.h"
#include <boost/any.hpp>
#include <map>
#include <string>

namespace Nitf
{

class NitfPropertiesManager : public ExecutableShell
{
public:
   NitfPropertiesManager();
   virtual ~NitfPropertiesManager();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   PlugInResource getPropertyPlugIn(const std::string& name, const DynamicObject& metadata);

private:
   void addProperties(Subject& subject, const std::string& signal, const boost::any& val);
   AttachmentPtr<DesktopServices> mpDesktop;
   std::map<std::string, std::string> mPropertiesPlugins;
};

}

#endif
