/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PICTURESPEXPORTER_H
#define PICTURESPEXPORTER_H

#include <QtGui/QImage>

#include "ExporterShell.h"
#include "PlugInManagerServices.h"
#include "TypesFile.h"

#include <string>

class PlotWidget;
class Progress;
class View;

class PicturesDetails
{
public:
   virtual ~PicturesDetails() {}
   virtual std::string name() = 0;
   virtual std::string shortDescription() = 0;
   virtual std::string description() = 0;
   virtual std::string extensions() = 0;
   virtual bool savePict(QString strFilename, QImage img, const SessionItem *pItem) = 0;
   virtual QWidget* getExportOptionsWidget(const PlugInArgList *pInArgList) { return NULL; }
   virtual bool isProduction() const = 0;
};

class PicturesExporter : public ExporterShell
{
public:
   PicturesExporter(PicturesDetails *pDetails);
   ~PicturesExporter();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   ValidationResultType validate(const PlugInArgList* pArgList, std::string& errorMessage) const;
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   QWidget* getExportOptionsWidget(const PlugInArgList *pInArgList);

protected:
   virtual bool extractInputArgs(const PlugInArgList* pInArgList);
   virtual bool generateImage(QImage &image) = 0;

   Progress* mpProgress;
   SessionItem *mpItem;

private:
   Service<PlugInManagerServices> mpPlugInManager;

   std::string mOutPath;
   std::string mMessage;
   std::auto_ptr<PicturesDetails> mpDetails;
};

#endif
