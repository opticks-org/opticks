/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSTIFFIMPORTER_H
#define OPTIONSTIFFIMPORTER_H

#include "LabeledSectionGroup.h"

class FileBrowser;
class RasterDataDescriptor;

class OptionsTiffImporter : public LabeledSectionGroup
{
   Q_OBJECT

public:
   OptionsTiffImporter(QWidget* pParent = NULL);
   virtual ~OptionsTiffImporter();

   void setDataDescriptor(RasterDataDescriptor* pDescriptor);
   RasterDataDescriptor* getDataDescriptor();
   const RasterDataDescriptor* getDataDescriptor() const;

   QString getFilename() const;
   void setFilename(const QString& filename);

protected slots:
   void loadIsdMetadata(const QString& filename);

private:
   OptionsTiffImporter(const OptionsTiffImporter& rhs);
   OptionsTiffImporter& operator=(const OptionsTiffImporter& rhs);

   RasterDataDescriptor* mpDescriptor;
   FileBrowser* mpFilename;
};

#endif
