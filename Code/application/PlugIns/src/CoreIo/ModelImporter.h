/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODELIMPORTER_H
#define MODELIMPORTER_H

#include "ImporterShell.h"
#include "ObjectFactory.h"
#include "xmlreader.h"

#include <map>
#include <string>
#include <vector>

class DataDescriptor;
class DesktopServices;
class ModelServices;
class PlugInManagerServices;
class QCheckBox;
class QWidget;

/**
 *  Model Importer
 *
 *  This plug-in imports core model elements.
 */
class ModelImporter : public ImporterShell
{
public:
   ModelImporter();
   ~ModelImporter();

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);

   static ImportDescriptor* populateImportDescriptor(XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement,
      const std::string& filename);

protected:
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename, bool reportErrors);

private:
   static std::map<std::string, std::string> sTypeMap;
   static std::string getTypeSubstitution(std::string type);

   QWidget* mpOptionsWidget;
   QCheckBox* mpCheckBox;

   Service<DesktopServices> mpDesktop;
   Service<ModelServices> mpModel;
   Service<PlugInManagerServices> mpPlugInManager;
};

#endif
