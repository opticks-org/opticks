/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DynamicObject.h"
#include "FileBrowser.h"
#include "FileDescriptor.h"
#include "LabeledSection.h"
#include "OptionsTiffImporter.h"
#include "QuickbirdIsd.h"
#include "RasterDataDescriptor.h"

#include <QtCore/QFileInfo>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

OptionsTiffImporter::OptionsTiffImporter(QWidget* pParent) :
   LabeledSectionGroup(pParent),
   mpDescriptor(NULL)
{
   QWidget* pFileBrowserWidget = new QWidget(this);
   QLabel* pFileBrowserLabel = new QLabel("Select an ISD metadata file or leave blank for no ISD metadata:",
      pFileBrowserWidget);
   mpFilename = new FileBrowser(pFileBrowserWidget);
   mpFilename->setWhatsThis("<p>An ISD metadata file is an XML file shipped with some imagery such as Quickbird. "
      "It contains image metadata. If you have an ISD metadata file and would like to load it with the cube data, "
      "select it here. If you do not want to load an ISD metadata file, leave this field blank.</p>"
      "Currently, the following information is loaded from the ISD file:"
      "<ul><li>RPC00B"
      "<li>Image header information needed to use the RPC00B"
      "</ul>");
   mpFilename->setBrowseCaption("Select ISD Metadata File");
   mpFilename->setBrowseFileFilters("ISD Metadata Files (*.xml)");
   mpFilename->setBrowseExistingFile(true);

   QGridLayout* pLayout = new QGridLayout(pFileBrowserWidget);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pFileBrowserLabel, 0, 0);
   pLayout->addWidget(mpFilename, 1, 0, 1, 2);
   pLayout->setColumnStretch(1, 5);

   LabeledSection* pFileBrowserSection = new LabeledSection(pFileBrowserWidget, "ISD Metadata File", this);

   // Layout
   QWidget* pWidget = widget();
   if (pWidget != NULL)
   {
      QLayout* pLayout = pWidget->layout();
      if (pLayout != NULL)
      {
         pLayout->setMargin(10);
      }
   }

   // Initialization
   addSection(pFileBrowserSection);
   addStretch(10);
   setSizeHint(300, 50);

   // Connections
   VERIFYNR(connect(mpFilename, SIGNAL(filenameChanged(const QString&)), this, SLOT(loadIsdMetadata(const QString&))));
}

OptionsTiffImporter::~OptionsTiffImporter()
{}

void OptionsTiffImporter::setDataDescriptor(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == mpDescriptor)
   {
      return;
   }

   mpDescriptor = pDescriptor;

   // Update the ISD filename
   QString isdFilename;
   if (mpDescriptor != NULL)
   {
      DynamicObject* pMetadata = mpDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         QuickbirdIsd isd(pMetadata);
         isdFilename = isd.getIsdFilename();
      }
   }

   setFilename(isdFilename);

   // Update the browse directory
   QString browseDir;
   if (mpDescriptor != NULL)
   {
      const FileDescriptor* pFileDescriptor = mpDescriptor->getFileDescriptor();
      if (pFileDescriptor != NULL)
      {
         browseDir = QString::fromStdString(pFileDescriptor->getFilename().getPath());
      }
   }

   mpFilename->setBrowseDirectory(browseDir);
}

RasterDataDescriptor* OptionsTiffImporter::getDataDescriptor()
{
   return mpDescriptor;
}

const RasterDataDescriptor* OptionsTiffImporter::getDataDescriptor() const
{
   return mpDescriptor;
}

QString OptionsTiffImporter::getFilename() const
{
   return mpFilename->getFilename();
}

void OptionsTiffImporter::setFilename(const QString &filename)
{
   mpFilename->setFilename(filename);
}

void OptionsTiffImporter::loadIsdMetadata(const QString& filename)
{
   if ((filename.isEmpty() == true) || (mpDescriptor == NULL))
   {
      return;
   }

   QuickbirdIsd isd(mpDescriptor->getMetadata());

   QFileInfo fileInfo(filename);
   if ((fileInfo.isFile() == false) || (fileInfo.exists() == false))
   {
      isd.removeIsdMetadata();
   }
   else
   {
      isd.loadIsdMetadata(filename);
   }
}
