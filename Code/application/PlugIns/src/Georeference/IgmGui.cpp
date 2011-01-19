/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FileBrowser.h"
#include "IgmGui.h"
#include "RasterElement.h"

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

IgmGui::IgmGui(RasterElement* pRasterElement, QWidget* pParent)
{

   QLabel* pFileBrowserLabel = new QLabel("Select an IGM file or leave blank for no IGM:", this);
   mpFilename = new FileBrowser(this);
   mpFilename->setToolTip("<p>An IGM file is an Image Geometry file used for georeferencing and georectification. ");
   mpFilename->setBrowseCaption("IGM File");
   mpFilename->setBrowseFileFilters("IGM Files (*.IGM *.HDR)");
   mpFilename->setBrowseExistingFile(true);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pFileBrowserLabel);
   pLayout->addWidget(mpFilename);
   pLayout->addStretch();
}

IgmGui::~IgmGui()
{
}

bool IgmGui::validateInput()
{
   return true;
}

QString IgmGui::getFilename() const
{
   return mpFilename->getFilename();
}
