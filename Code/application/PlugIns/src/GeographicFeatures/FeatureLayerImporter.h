/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURELAYERIMPORTER_H
#define FEATURELAYERIMPORTER_H

#include "ShapeFileImporter.h"

class FeatureLayerImporter : public ShapeFileImporter
{
public:
   FeatureLayerImporter();
   ~FeatureLayerImporter();

   unsigned char getFileAffinity(const std::string& filename);
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   static const std::string FEATURE_DYNAMIC_OBJECT_ARG;
   static const std::string PLUGIN_NAME;
   static const std::string FILE_PLACEHOLDER;

protected:
   virtual void createFeatureClassIfNeeded(const DataDescriptor *pDescriptor);
   virtual std::vector<ArcProxyLib::ConnectionType> getAvailableConnectionTypes();

private:
   DynamicObject* mpFeatureDynObj;
};

#endif
