/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFDESEXPORTER_H__
#define NITFDESEXPORTER_H__

#include "ExporterShell.h"

class QComboBox;
class QWidget;

namespace Nitf
{
class DesExporter : public ExporterShell
{
public:
   DesExporter();
   virtual ~DesExporter();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual ValidationResultType validate(const PlugInArgList* pArgList, std::string& errorMessage) const;
   virtual QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   QWidget* mpOptions;
   QComboBox* mpCombo;
};
}

#endif