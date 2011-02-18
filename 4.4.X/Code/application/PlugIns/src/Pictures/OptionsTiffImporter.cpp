/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FileBrowser.h"
#include "LabeledSection.h"
#include "OptionsTiffImporter.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

OptionsTiffImporter::OptionsTiffImporter(const QString &initialDirectory) : LabeledSectionGroup(NULL)
{
   QWidget* pFileBrowserWidget = new QWidget(this);
   QLabel* pFileBrowserLabel = new QLabel("Select an ISD metadata file or leave blank for no ISD metadata:",
      pFileBrowserWidget);
   mpFilename = new FileBrowser(pFileBrowserWidget);
   mpFilename->setWhatsThis("<p>An ISD metadata file is an XML file shipped with some imagery such as Quickbird. "
                            "It contains image metadata. If you have an ISD metadata file and would like to load "
                            "it with the cube data, select it here. If you do not want to load an ISD metadata file, "
                            "leave this field blank.</p>"
                            "Currently, the following information is loaded from the ISD file:"
                            "<ul><li>RPC00B"
                            "<li>Image header information needed to use the RPC00B"
                            "</ul>");
   mpFilename->setBrowseCaption("ISD Metadata File");
   mpFilename->setBrowseFileFilters("ISD Metadata Files (*.xml)");
   mpFilename->setBrowseExistingFile(true);
   if (!initialDirectory.isEmpty())
   {
      mpFilename->setBrowseDirectory(initialDirectory);
   }
   QGridLayout* pLayout = new QGridLayout(pFileBrowserWidget);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pFileBrowserLabel, 0, 0);
   pLayout->addWidget(mpFilename, 1, 0, 1, 2);
   pLayout->setColumnStretch(1, 5);

   LabeledSection* pFileBrowserSection = new LabeledSection(pFileBrowserWidget, "ISD Metadata File", this);

   // Initialization
   addSection(pFileBrowserSection);
   addStretch(10);
   setSizeHint(300, 50);
}

OptionsTiffImporter::~OptionsTiffImporter()
{
}

QString OptionsTiffImporter::getFilename() const
{
   return mpFilename->getFilename();
}

void OptionsTiffImporter::setFilename(const QString &filename)
{
   mpFilename->setFilename(filename);
}
