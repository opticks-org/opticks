/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "FileBrowser.h"
#include "IgmGui.h"
#include "RasterElement.h"

#include <QtGui/QGridLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QStyleOptionButton>

IgmGui::IgmGui(RasterElement* pRasterElement, QWidget* pParent)
{
   mpUseExisting = new QRadioButton("Use existing IGM", this);
   mpLoad = new QRadioButton("Load an IGM file:", this);
   mpFilename = new FileBrowser(this);
   mpFilename->setToolTip("<p>An IGM file is an Image Geometry file used for georeferencing and georectification. ");
   mpFilename->setBrowseCaption("IGM File");
   mpFilename->setBrowseFileFilters("IGM Files (*.IGM *.HDR)");
   mpFilename->setBrowseExistingFile(true);

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpUseExisting, 0, 0, 1, 2);
   pLayout->addWidget(mpLoad, 1, 0, 1, 2);
   pLayout->addWidget(mpFilename, 2, 1);
   QStyleOptionButton option;
   option.initFrom(mpLoad);
   int radioWidth = style()->subElementRect(QStyle::SE_RadioButtonIndicator, &option).width();
   pLayout->setColumnMinimumWidth(0, radioWidth);
   pLayout->setRowStretch(3, 10);
   pLayout->setColumnStretch(1, 10);

   VERIFYNR(connect(mpLoad, SIGNAL(toggled(bool)), mpFilename, SLOT(setEnabled(bool))));
   mpUseExisting->setEnabled(false);
   mpLoad->setChecked(true);
}

IgmGui::~IgmGui()
{
}

bool IgmGui::validateInput()
{
   return useExisting() || !getFilename().isEmpty();
}

bool IgmGui::useExisting() const
{
   return mpUseExisting->isChecked();
}

void IgmGui::hasExisting(bool val)
{
   mpUseExisting->setEnabled(val);
   mpUseExisting->setChecked(val);
}

QString IgmGui::getFilename() const
{
   return mpFilename->getFilename();
}
