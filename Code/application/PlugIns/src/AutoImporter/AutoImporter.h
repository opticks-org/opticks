/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AUTOIMPORTER_H
#define AUTOIMPORTER_H

#include "ImporterShell.h"
#include "PlugInManagerServices.h"

#include <map>
#include <string>
#include <vector>

class DataElement;
class Importer;
class PlugIn;
class Progress;

class AutoImporter : public ImporterShell
{
public:
   AutoImporter();
   ~AutoImporter();

   std::string getDefaultExtensions() const;
   bool isProcessingLocationSupported(ProcessingLocation location) const;
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   QWidget* getPreview(const DataDescriptor* pDescriptor, Progress* pProgress);
   bool validate(const DataDescriptor* pDescriptor, const std::vector<const DataDescriptor*>& importedDescriptors,
      std::string& errorMessage) const;
   QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);
   void polishDataDescriptor(DataDescriptor* pDescriptor);

   bool setBatch();
   bool setInteractive();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool hasAbort();
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   bool extractPlugInArgs(const PlugInArgList* pArgList);
   bool checkExtension(const PlugInDescriptor* pDescriptor, const std::string& filename) const;
   Importer* findImporter(const DataDescriptor* pDescriptor);
   Importer* findImporter(const std::string& filename);

private:
   bool mbInteractive;
   Service<PlugInManagerServices> mpPlugInManager;

   std::string mMenuCommand;
   Progress* mpProgress;
   DataElement* mpElement;

   PlugIn* mpPlugIn;
   std::map<std::string, std::pair<std::string, unsigned char> > mFilenames;
   std::map<std::string, PlugIn*> mPlugIns;
};

#endif
