/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGIN_RESOURCE_DOT_H
#define PLUGIN_RESOURCE_DOT_H

#include "ExecutableAgent.h"
#include "ExportAgent.h"
#include "ImportAgent.h"
#include "ObjectResource.h"
#include "PlugIn.h"
#include "PlugInManagerServices.h"
#include "Resource.h"

#include <algorithm>
#include <string>

/**
 * The %PlugInObject is a trait object for use with the %Resource template. 
 *
 * The %PlugInObject is a trait object for use with the %Resource template. It provides capability for
 * getting and destroying plug-in instances.
 * 
 * @see PlugInResource
 */
class PlugInObject
{
public:
   class Args 
   {
   public:
      std::string mPlugInName;
      PlugIn* mpPlugIn;
      bool mNeedToCreate;
      Args() : mpPlugIn(NULL), mNeedToCreate(false) {}
      Args(std::string plugInName) : mPlugInName(plugInName), mpPlugIn(NULL), mNeedToCreate(true) {}
      Args(char* plugInName) : mPlugInName(plugInName), mpPlugIn(NULL), mNeedToCreate(true) {}
      Args(PlugIn* pPlugIn) : mpPlugIn(pPlugIn), mNeedToCreate(false) {}
   };

   PlugIn* obtainResource(const Args& args) const
   {
      if (args.mpPlugIn != NULL)
      {
         return args.mpPlugIn;
      }
      if (!args.mNeedToCreate)
      {
         return NULL;
      }
      Service<PlugInManagerServices> pPlugInManager;
      PlugIn* pPlugIn = pPlugInManager->createPlugIn(args.mPlugInName);
      return pPlugIn;
   }

   void releaseResource(const Args& args, PlugIn* pPlugIn) const
   {
      Service<PlugInManagerServices> pPlugInManager;
      pPlugInManager->destroyPlugIn(pPlugIn);
   }
};

/**
 *  This is a %Resource class that wraps a plug-in.
 *
 *  This is a specialized %Resource class that wraps a plug-in to ensure
 *  proper creation and destruction of the plug-in.  When the %PlugInResource
 *  object goes out of scope, the plug-in will be destroyed.
 *
 *  @see     Resource, PlugInObject
 */
class PlugInResource : public Resource<PlugIn, PlugInObject>
{
public:
   /**
    *  Creates a plug-in resource which wraps no plug-in.
    *
    */
   explicit PlugInResource() :
      Resource<PlugIn, PlugInObject>(PlugInObject::Args())
   {
   }

   /**
    *  Creates a plug-in resource to create a plug-in with a given
    *  name.
    *
    *  @param   plugInName
    *           The name of the plug-in to create.
    */
   explicit PlugInResource(const std::string& plugInName) :
      Resource<PlugIn, PlugInObject>(PlugInObject::Args(plugInName))
   {
   }

   /**
    *  Creates a plug-in resource to manage an existing plug-in.
    *
    *  @param   pPlugIn
    *           The plug-in to manage. The resource assumes ownership of the
    *           given plug-in and will destroy it as necessary upon resource
    *           destruction.
    */
   explicit PlugInResource(PlugIn* pPlugIn) :
      Resource<PlugIn, PlugInObject>(pPlugIn, PlugInObject::Args(pPlugIn))
   {
   } 
};

/**
 *  This is helper class when using the Executable interface.  This
 *  class manages an instance of an ExecutableAgent that provides
 *  the actual helper methods when working with instances of
 *  Executable plug-ins.
 *
 *  @see ExecutableAgent, Executable.
 */
class ExecutableResource : public FactoryResource<ExecutableAgentCommon>
{
public:
   /**
    * @copydoc ExecutableAgent::instantiate(Progress*,bool)
    * @par ExecutableAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExecutableAgent object.  You can
    * use -> to access any of the methods available
    * on ExecutableAgent.
    */
   explicit ExecutableResource(Progress* pProgress = NULL, bool batch = true)
   {
      ExecutableAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(pProgress, batch);
      }
   }

   /**
    * @copydoc ExecutableAgent::instantiate(const std::string&,const std::string&,Progress*,bool)
    * @par ExecutableAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExecutableAgent object.  You can
    * use -> to access any of the methods available
    * on ExecutableAgent.
    */
   explicit ExecutableResource(const std::string& plugInName, const std::string& menuCommand = std::string(),
      Progress* pProgress = NULL, bool batch = true)
   {
      ExecutableAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(plugInName, menuCommand, pProgress, batch);
      }
   }

   /**
    * @copydoc ExecutableAgent::instantiate(PlugIn*,const std::string&,Progress*,bool)
    * @par ExecutableAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExecutableAgent object.  You can
    * use -> to access any of the methods available
    * on ExecutableAgent.
    */
   explicit ExecutableResource(PlugIn* pPlugIn, const std::string& menuCommand = std::string(),
      Progress* pProgress = NULL, bool batch = true)
   {
      ExecutableAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(pPlugIn, menuCommand, pProgress, batch);
      }
   }
};

/**
 *  This is helper class when using the Importer interface.  This
 *  class manages an instance of an ImportAgent that provides
 *  the actual helper methods when working with instances of
 *  Importer plug-ins.
 *
 *  @see ImportAgent, Importer, \ref usingimporterresource "Using ImporterResource"
 */
class ImporterResource : public FactoryResource<ImportAgentCommon>
{
public:
   /**
    * @copydoc ImportAgent::instantiate(Progress*,bool)
    * @par ImportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ImportAgent object.  You can
    * use -> to access any of the methods available
    * on ImportAgent.
    */
   ImporterResource(Progress* pProgress = NULL, bool batch = true)
   {
      ImportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(pProgress, batch);
      }
   }

   /**
    * @copydoc ImportAgent::instantiate(const std::string&,Progress*,bool)
    * @par ImportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ImportAgent object.  You can
    * use -> to access any of the methods available
    * on ImportAgent.
    */
   ImporterResource(const std::string& importerName, Progress* pProgress = NULL, bool batch = true)
   {
      ImportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(importerName, pProgress, batch);
      }
   }

   /**
    * @copydoc ImportAgent::instantiate(const std::string&,const std::string&,Progress*,bool)
    * @par ImportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ImportAgent object.  You can
    * use -> to access any of the methods available
    * on ImportAgent.
    */
   ImporterResource(const std::string& importerName, const std::string& filename, Progress* pProgress = NULL, bool batch = true)
   {
      ImportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(importerName, filename, pProgress, batch);
      }
   }

   /**
    * @copydoc ImportAgent::instantiate(const std::string&,const std::vector<ImportDescriptor*>&,Progress*,bool)
    * @par ImportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ImportAgent object.  You can
    * use -> to access any of the methods available
    * on ImportAgent.
    */
   ImporterResource(const std::string& importerName, const std::vector<ImportDescriptor*>& descriptors,
                    Progress* pProgress = NULL, bool batch = true)
   {
      ImportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(importerName, descriptors, pProgress, batch);
      }
   }

   /**
    * @copydoc ImportAgent::instantiate(PlugIn*,const std::vector<ImportDescriptor*>&,Progress*,bool)
    * @par ImportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ImportAgent object.  You can
    * use -> to access any of the methods available
    * on ImportAgent.
    */
   ImporterResource(PlugIn* pPlugIn, const std::vector<ImportDescriptor*>& descriptors, Progress* pProgress = NULL,
                    bool batch = true)
   {
      ImportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(pPlugIn, descriptors, pProgress, batch);
      }
   }
};

/**
 *  This is helper class when using the Exporter interface.  This
 *  class manages an instance of an ExportAgent that provides
 *  the actual helper methods when working with instances of
 *  Exporter plug-ins.
 *
 *  @see ExportAgent, Exporter.
 */
class ExporterResource : public FactoryResource<ExportAgentCommon>
{
public:
   /**
    * @copydoc ExportAgent::instantiate(Progress*,bool)
    * @par ExportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExportAgent object.  You can
    * use -> to access any of the methods available
    * on ExportAgent.
    */
   ExporterResource(Progress* pProgress = NULL, bool batch = true)
   {
      ExportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(pProgress, batch);
      }
   }

   /**
    * @copydoc ExportAgent::instantiate(std::string,Progress*,bool)
    * @par ExportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExportAgent object.  You can
    * use -> to access any of the methods available
    * on ExportAgent.
    */
   ExporterResource(std::string exporterName, Progress* pProgress = NULL, bool batch = true)
   {
      ExportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(exporterName, pProgress, batch);
      }
   }

   /**
    * @copydoc ExportAgent::instantiate(PlugIn*,Progress*,bool)
    * @par ExportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExportAgent object.  You can
    * use -> to access any of the methods available
    * on ExportAgent.
    */
   ExporterResource(PlugIn* pPlugIn, Progress* pProgress = NULL, bool batch = true)
   {
      ExportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(pPlugIn, pProgress, batch);
      }
   }

   /**
    * @copydoc ExportAgent::instantiate(std::string,SessionItem*,FileDescriptor*,Progress*,bool)
    * @par ExportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExportAgent object.  You can
    * use -> to access any of the methods available
    * on ExportAgent.
    */
   ExporterResource(std::string exporterName, SessionItem* pItem, FileDescriptor* pFileDescriptor,
      Progress* pProgress = NULL, bool batch = true)
   {
      ExportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(exporterName, pItem, pFileDescriptor, pProgress, batch);
      }
   }

   /**
    * @copydoc ExportAgent::instantiate(PlugIn*,SessionItem*,FileDescriptor*,Progress*,bool)
    * @par ExportAgent
    * This class simply allocates and controls the 
    * lifecycle of a ExportAgent object.  You can
    * use -> to access any of the methods available
    * on ExportAgent.
    */
   ExporterResource(PlugIn* pPlugIn, SessionItem* pItem, FileDescriptor* pFileDescriptor,
      Progress* pProgress = NULL, bool batch = true)
   {
      ExportAgentCommon* pAgent = get();
      if (pAgent != NULL)
      {
         pAgent->instantiate(pPlugIn, pItem, pFileDescriptor, pProgress, batch);
      }
   }
};

#endif
