/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTAGENTIMP_H
#define IMPORTAGENTIMP_H

#include <QtCore/QMap>

#include "ExecutableAgentImp.h"

#include <map>
#include <string>
#include <vector>

class DataElement;
class ImportDescriptor;
class Importer;
class PlugInArgList;

class ImportAgentImp : public ExecutableAgentImp
{
public:
   using ExecutableAgentImp::instantiate;
   void instantiate(Progress* pProgress, bool batch);
   void instantiate(const std::string& importerName, Progress* pProgress, bool batch);
   void instantiate(const std::string& importerName, const std::string& filename, Progress* pProgress,
      bool batch);
   void instantiate(const std::string& importerName, const std::vector<std::string>& filenames, Progress* pProgress,
      bool batch);
   void instantiate(const std::string& importerName,
      const std::map<std::string, std::vector<ImportDescriptor*> >& descriptors, Progress* pProgress, bool batch);
   void instantiate(PlugIn* pPlugIn, const std::map<std::string, std::vector<ImportDescriptor*> >& descriptors,
      Progress* pProgress, bool batch);

   ImportAgentImp();
   ~ImportAgentImp();

   void setImporterSubtype(const std::string& subtype);
   std::string getImporterSubtype() const;

   void setFilename(const std::string& filename);
   void setFilenames(const std::vector<std::string>& filenames);
   void setDatasets(const std::map<std::string, std::vector<ImportDescriptor*> >& datasets);
   std::vector<ImportDescriptor*> getImportDescriptors();
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   std::map<std::string, std::vector<ImportDescriptor*> > getDatasets();

   void setEditType(ImportAgent::EditType editType = ImportAgent::AS_NEEDED_EDIT);
   ImportAgent::EditType getEditType() const;
   void updateMruFileList(bool updateList);
   bool isMruFileListUpdated() const;
   std::string getDefaultExtensions() const;
   bool execute();
   std::vector<DataElement*> getImportedElements() const;

   static unsigned int validateImportDescriptors(const std::vector<ImportDescriptor*>& descriptors,
      Importer* pImporter, std::string& errorMessage);

protected:
   void populateArgValues(PlugInArgList *pArgList);
   void updateMruFileList(const std::map<ImportDescriptor*, bool>& importedDescriptors);

private:
   std::string mImporterSubtype;
   QMap<QString, std::vector<ImportDescriptor*> > mDatasets;
   ImportAgent::EditType mEditType;
   bool mUpdateMruList;
   DataElement* mpElement;
   std::vector<DataElement*> mImportedElements;
};

#define IMPORTAGENTADAPTEREXTENSION_CLASSES \
   EXECUTABLEAGENTADAPTEREXTENSION_CLASSES

#define IMPORTAGENTADAPTER_METHODS(impClass) \
   EXECUTABLEAGENTADAPTER_METHODS(impClass) \
   void instantiate(const std::string& importerName, Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(importerName, pProgress, batch); \
   } \
   void instantiate(const std::string& importerName, const std::vector<std::string>& filenames, Progress* pProgress, \
                    bool batch) \
   { \
      impClass::instantiate(importerName, filenames, pProgress, batch); \
   } \
   void instantiate(const std::string& importerName, \
                    const std::map<std::string, std::vector<ImportDescriptor*> >& descriptors, Progress* pProgress, \
                    bool batch) \
   { \
      impClass::instantiate(importerName, descriptors, pProgress, batch); \
   } \
   void instantiate(PlugIn* pPlugIn, const std::map<std::string, std::vector<ImportDescriptor*> >& descriptors, \
                    Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(pPlugIn, descriptors, pProgress, batch); \
   } \
   void setImporterSubtype(const std::string& subtype) \
   { \
      impClass::setImporterSubtype(subtype); \
   } \
   std::string getImporterSubtype() const \
   { \
      return impClass::getImporterSubtype(); \
   } \
   void setFilename(const std::string& filename) \
   { \
      impClass::setFilename(filename); \
   } \
   void setFilenames(const std::vector<std::string>& filenames) \
   { \
      impClass::setFilenames(filenames); \
   } \
   void setEditType(ImportAgent::EditType editType = ImportAgent::AS_NEEDED_EDIT) \
   { \
      impClass::setEditType(editType); \
   } \
   ImportAgent::EditType getEditType() const \
   { \
      return impClass::getEditType(); \
   } \
   void updateMruFileList(bool updateList) \
   { \
      impClass::updateMruFileList(updateList); \
   } \
   bool isMruFileListUpdated() const \
   { \
      return impClass::isMruFileListUpdated(); \
   } \
   void setDatasets(const std::map<std::string, std::vector<ImportDescriptor*> >& datasets) \
   { \
      impClass::setDatasets(datasets); \
   } \
   std::vector<ImportDescriptor*> getImportDescriptors() \
   { \
      return impClass::getImportDescriptors(); \
   } \
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename) \
   { \
      return impClass::getImportDescriptors(filename); \
   } \
   std::map<std::string, std::vector<ImportDescriptor*> > getDatasets() \
   { \
      return impClass::getDatasets(); \
   } \
   std::string getDefaultExtensions() const \
   { \
      return impClass::getDefaultExtensions(); \
   } \
   std::vector<DataElement*> getImportedElements() const \
   { \
      return impClass::getImportedElements(); \
   }

#endif
