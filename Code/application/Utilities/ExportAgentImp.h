/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXPORT_AGENT_IMP_H
#define EXPORT_AGENT_IMP_H

#include "ExecutableAgentImp.h"
#include "TypesFile.h"

#include <string>

class DataElement;
class Exporter;
class FileDescriptor;
class Layer;
class PlotWidget;
class PlugInArgList;
class QWidget;
class View;

class ExportAgentImp : public ExecutableAgentImp
{
public:
   using ExecutableAgentImp::instantiate;
   virtual void instantiate(Progress* pProgress, bool batch);
   virtual void instantiate(std::string exporterName, Progress* pProgress, bool batch);
   virtual void instantiate(PlugIn* pPlugIn, Progress* pProgress, bool batch);
   virtual void instantiate(std::string exporterName, SessionItem* pItem, FileDescriptor* pFileDescriptor,
      Progress* pProgress, bool batch);
   virtual void instantiate(PlugIn* pPlugIn, SessionItem* pItem, FileDescriptor* pFileDescriptor,
      Progress* pProgress, bool batch);

   ExportAgentImp();
   virtual ~ExportAgentImp();

   virtual Exporter *getExporter();
   virtual const Exporter *getExporter() const;
   virtual QWidget *getExportOptionsWidget();
   virtual void setItem(SessionItem* pItem);
   virtual SessionItem* getItem() const;
   virtual void setFileDescriptor(FileDescriptor* pFileDescriptor);
   virtual FileDescriptor* getFileDescriptor();
   virtual const FileDescriptor* getFileDescriptor() const;
   virtual std::string getDefaultExtensions() const;
   virtual ValidationResultType validate(std::string &errorMessage);

protected:
   void populateArgValues(PlugInArgList *pArgList);

private:
   SessionItem *mpItem;
   FileDescriptor* mpFileDescriptor;
};

#define EXPORTAGENTADAPTEREXTENSION_CLASSES \
   EXECUTABLEAGENTADAPTEREXTENSION_CLASSES

#define EXPORTAGENTADAPTER_METHODS(impClass) \
   EXECUTABLEAGENTADAPTER_METHODS(impClass) \
   void instantiate(std::string exporterName, Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(exporterName, pProgress, batch); \
   } \
   void instantiate(PlugIn* pPlugIn, Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(pPlugIn, pProgress, batch); \
   } \
   void instantiate(std::string exporterName, SessionItem *pItem, FileDescriptor* pFileDescriptor, \
      Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(exporterName, pItem, pFileDescriptor, pProgress, batch); \
   } \
   void instantiate(PlugIn* pPlugIn, SessionItem *pItem, FileDescriptor* pFileDescriptor, \
      Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(pPlugIn, pItem, pFileDescriptor, pProgress, batch); \
   } \
   Exporter *getExporter() \
   { \
      return impClass::getExporter(); \
   } \
   const Exporter *getExporter() const \
   { \
      return impClass::getExporter(); \
   } \
   QWidget *getExportOptionsWidget() \
   { \
      return impClass::getExportOptionsWidget(); \
   } \
   void setItem(SessionItem* pItem) \
   { \
      impClass::setItem(pItem); \
   } \
   SessionItem* getItem() const \
   { \
      return impClass::getItem(); \
   } \
   void setFileDescriptor(FileDescriptor* pFileDescriptor) \
   { \
      impClass::setFileDescriptor(pFileDescriptor); \
   } \
   FileDescriptor* getFileDescriptor() \
   { \
      return impClass::getFileDescriptor(); \
   } \
   const FileDescriptor* getFileDescriptor() const \
   { \
      return impClass::getFileDescriptor(); \
   } \
   std::string getDefaultExtensions() const \
   { \
      return impClass::getDefaultExtensions(); \
   } \
   ValidationResultType validate(std::string &errorMessage) \
   { \
      return impClass::validate(errorMessage); \
   }

#endif
