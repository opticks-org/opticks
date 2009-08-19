/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#include "ExportAgentImp.h"

#include "Exporter.h"
#include "FileDescriptor.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include <stdexcept>

void ExportAgentImp::instantiate(Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw std::logic_error("ExportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(pProgress, batch);
   mpItem = NULL;
   mpFileDescriptor = NULL;
}

void ExportAgentImp::instantiate(std::string exporterName, Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw std::logic_error("ExportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(exporterName, std::string(), pProgress, batch);
   mpItem = NULL;
   mpFileDescriptor = NULL;
}

void ExportAgentImp::instantiate(PlugIn* pPlugIn, Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw std::logic_error("ExportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(pPlugIn, std::string(), pProgress, batch);
   mpItem = NULL;
   mpFileDescriptor = NULL;
}

void ExportAgentImp::instantiate(std::string exporterName, SessionItem* pItem, FileDescriptor* pFileDescriptor,
   Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw std::logic_error("ExportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(exporterName, std::string(), pProgress, batch);
   mpItem = pItem;
   mpFileDescriptor = pFileDescriptor;
}

void ExportAgentImp::instantiate(PlugIn* pPlugIn, SessionItem* pItem, FileDescriptor* pFileDescriptor,
   Progress* pProgress, bool batch)
{
   if (getInstantiated())
   {
      throw std::logic_error("ExportAgent can not be instantiated twice!");
   }

   ExecutableAgentImp::instantiate(pPlugIn, std::string(), pProgress, batch);
   mpItem = pItem;
   mpFileDescriptor = pFileDescriptor;
}

ExportAgentImp::ExportAgentImp() :
   mpItem(NULL),
   mpFileDescriptor(NULL)
{
}

ExportAgentImp::~ExportAgentImp()
{
}

Exporter* ExportAgentImp::getExporter()
{
   checkInstantiate();
   return dynamic_cast<Exporter*>(getPlugIn());
}

const Exporter* ExportAgentImp::getExporter() const
{
   checkInstantiate();
   return dynamic_cast<const Exporter*>(getPlugIn());
}

QWidget* ExportAgentImp::getExportOptionsWidget()
{
   checkInstantiate();
   QWidget* pWidget = NULL;

   Exporter* pExporter = getExporter();
   if (pExporter != NULL)
   {
      pWidget = pExporter->getExportOptionsWidget(&getPopulatedInArgList());
   }

   return pWidget;
}

void ExportAgentImp::setItem(SessionItem* pItem)
{
   checkInstantiate();
   mpItem = pItem;
}

SessionItem* ExportAgentImp::getItem() const
{
   checkInstantiate();
   return mpItem;
}

void ExportAgentImp::setFileDescriptor(FileDescriptor* pFileDescriptor)
{
   checkInstantiate();
   mpFileDescriptor = pFileDescriptor;
}

FileDescriptor* ExportAgentImp::getFileDescriptor()
{
   checkInstantiate();
   return mpFileDescriptor;
}

const FileDescriptor* ExportAgentImp::getFileDescriptor() const
{
   checkInstantiate();
   return mpFileDescriptor;
}

std::string ExportAgentImp::getDefaultExtensions() const
{
   checkInstantiate();
   std::string extensions;
   const Exporter* pExporter = dynamic_cast<const Exporter*>(getPlugIn());
   if (pExporter != NULL)
   {
      extensions = pExporter->getDefaultExtensions();
   }

   return extensions;
}

ValidationResultType ExportAgentImp::validate(std::string &errorMessage)
{
   checkInstantiate();
   setupArgList();
   PlugInArgList& inArgList = getPopulatedInArgList();
   Exporter* pExporter = getExporter();
   ValidationResultType result = VALIDATE_FAILURE;
   if (pExporter != NULL)
   {
      result = pExporter->validate(&inArgList, errorMessage);
   }
   return result;
}

void ExportAgentImp::populateArgValues(PlugInArgList *pArgList)
{
   checkInstantiate();
   if (pArgList != NULL)
   {
      ExecutableAgentImp::populateArgValues(pArgList);

      // Session item
      PlugInArg* pSessionItemArg = NULL;
      pArgList->getArg(Exporter::ExportItemArg(), pSessionItemArg);
      if (pSessionItemArg != NULL)
      {
         if (pSessionItemArg->isActualSet() == false)
         {
            pSessionItemArg->setPlugInArgValueLoose(mpItem);
         }
      }

      // File descriptor
      PlugInArg* pFileDescriptorArg = NULL;
      pArgList->getArg(Exporter::ExportDescriptorArg(), pFileDescriptorArg);
      if (pFileDescriptorArg != NULL)
      {
         if (pFileDescriptorArg->isActualSet() == false)
         {
            pFileDescriptorArg->setPlugInArgValueLoose(mpFileDescriptor);
         }
      }
   }
}
