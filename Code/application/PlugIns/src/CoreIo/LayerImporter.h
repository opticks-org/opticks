/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYERIMPORTER_H
#define LAYERIMPORTER_H

#include "ImporterShell.h"

#include <boost/atomic.hpp>
#include <memory>

class DesktopServices;
class PlugInManagerServices;

/**
 *  Layer Importer
 *
 *  This plug-in imports layers.
 */
class LayerImporter : public ImporterShell
{
public:
   LayerImporter();
   virtual ~LayerImporter();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual unsigned char getFileAffinity(const std::string& filename);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   virtual QWidget* getImportOptionsWidget(DataDescriptor* pDescriptor);

protected:
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename, bool reportErrors);
   virtual void polishDataDescriptor (DataDescriptor *pDescriptor);

private:
   bool removeGeoNodes(DOMNode* pNode) const;

   std::auto_ptr<QWidget> mpOptionsWidget;
   QCheckBox* mpCheckBox;

   Service<DesktopServices> mpDesktop;
   Service<PlugInManagerServices> mpPlugInManager;

   boost::atomic<bool> mPolishEntered; // used to prevent infinite recursion in polishDataDescriptor
};

#endif
