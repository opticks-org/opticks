/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORT_AGENT_IMP_H
#define IMPORT_AGENT_IMP_H

#include "ExecutableAgentImp.h"
#include <string>
#include <vector>

class DataDescriptor;
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
   void instantiate(const std::string& importerName, const std::vector<ImportDescriptor*>& descriptors,
      Progress* pProgress, bool batch);
   void instantiate(PlugIn* pPlugIn, const std::vector<ImportDescriptor*>& descriptors, Progress* pProgress,
      bool batch);

   ImportAgentImp();
   ~ImportAgentImp();

   void setImporterSubtype(const std::string& subtype);
   std::string getImporterSubtype() const;
   void setFilename(const std::string& filename);
   void setEditType(ImportAgentExt1::EditType editType = ImportAgentExt1::AS_NEEDED_EDIT);
   ImportAgentExt1::EditType getEditType() const;
   void updateMruFileList(bool updateList);
   bool isMruFileListUpdated() const;
   void setImportDescriptors(const std::vector<ImportDescriptor*>& descriptors);
   std::vector<ImportDescriptor*> getImportDescriptors();
   std::string getDefaultExtensions() const;
   bool execute();
   std::vector<DataElement*> getImportedElements() const;

   static unsigned int validateImportDescriptors(const std::vector<ImportDescriptor*>& descriptors,
      Importer* pImporter, std::string& errorMessage);

protected:
   void populateArgValues(PlugInArgList *pArgList);
   void updateMruFileList();

private:
   std::string mImporterSubtype;
   std::string mFilename;
   std::vector<ImportDescriptor*> mDescriptors;
   ImportAgentExt1::EditType mEditType;
   bool mUpdateMruList;
   DataElement* mpElement;
   std::vector<DataElement*> mImportedElements;
};

#define IMPORTAGENTADAPTEREXTENSION_CLASSES \
   EXECUTABLEAGENTADAPTEREXTENSION_CLASSES

#define IMPORTAGENTADAPTER_METHODS(impClass) \
   EXECUTABLEAGENTADAPTER_METHODS(impClass) \
   void instantiate(const std::string& importerName, const std::vector<ImportDescriptor*>& descriptors, \
                    Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(importerName, descriptors, pProgress, batch); \
   } \
   void instantiate(const std::string& importerName, Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(importerName, pProgress, batch); \
   } \
   void instantiate(PlugIn* pPlugIn, const std::vector<ImportDescriptor*>& descriptors, Progress* pProgress, \
                    bool batch) \
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
   void showOptionsDialog(bool showDialog) \
   { \
      impClass::setEditType(showDialog ? ImportAgentExt1::ALWAYS_EDIT : ImportAgentExt1::NEVER_EDIT); \
   } \
   bool isOptionsDialogShown() const \
   { \
      ImportAgentExt1::EditType editType = impClass::getEditType(); \
      return (editType == ImportAgentExt1::ALWAYS_EDIT); \
   } \
   void setEditType(ImportAgentExt1::EditType editType = ImportAgentExt1::AS_NEEDED_EDIT) \
   { \
      impClass::setEditType(editType); \
   } \
   ImportAgentExt1::EditType getEditType() const \
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
   void setImportDescriptors(const std::vector<ImportDescriptor*>& descriptors) \
   { \
      impClass::setImportDescriptors(descriptors); \
   } \
   std::vector<ImportDescriptor*> getImportDescriptors() \
   { \
      return impClass::getImportDescriptors(); \
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
