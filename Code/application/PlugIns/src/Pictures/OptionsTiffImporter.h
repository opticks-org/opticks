/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSTIFFIMPORTER_H__
#define OPTIONSTIFFIMPORTER_H__

#include "LabeledSectionGroup.h"

class FileBrowser;

class OptionsTiffImporter : public LabeledSectionGroup
{
   Q_OBJECT
   Q_PROPERTY(QString filename READ getFilename WRITE setFilename)

public:
   OptionsTiffImporter(const QString &initialDirectory=QString());
   virtual ~OptionsTiffImporter();

   QString getFilename() const;
   void setFilename(const QString &filename);

private:
   FileBrowser *mpFilename;
};

#endif
