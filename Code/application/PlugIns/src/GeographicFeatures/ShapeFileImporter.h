/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SHAPEFILEIMPORTER_H
#define SHAPEFILEIMPORTER_H

#include "ConnectionParameters.h"
#include "ImporterShell.h"

#include <memory>
#include <string>

class FeatureClass;
class FeatureClassWidget;
class Step;

class ShapeFileImporter : public ImporterShell
{
public:
   ShapeFileImporter(void);
   ~ShapeFileImporter(void);

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   bool validate(const DataDescriptor *pDescriptor, std::string &errorMessage) const;

   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   unsigned char getFileAffinity(const std::string& filename);
   QWidget *getImportOptionsWidget(DataDescriptor *pDescriptor);

   static const std::string PLUGIN_NAME;
   static const std::string PLUGIN_SUBTYPE;

protected:
   virtual void createFeatureClassIfNeeded(const DataDescriptor *pDescriptor);
   virtual std::vector<ArcProxyLib::ConnectionType> getAvailableConnectionTypes();
   
   std::auto_ptr<FeatureClass> mpFeatureClass;
   mutable std::string mMessageText;

   Progress* mpProgress;
   Step* mpStep;

private:
   FeatureClassWidget* mpOptionsWidget;
};

#endif
