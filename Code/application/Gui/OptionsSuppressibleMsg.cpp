/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsSuppressibleMsg.h"

#include "DesktopServices.h"
#include "LabeledSection.h"
#include "MetadataWidget.h"
#include "PropertiesRasterLayer.h"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

OptionsSuppressibleMsg::OptionsSuppressibleMsg() :
   QWidget(NULL)
{
   mpEditMetadataDialogState = new QCheckBox("Edit metadata", this);
   mpEnableDisplayAsDialogState = new QCheckBox("Warn when \"Display As\" menu commands fail", this);
   mpEnableTextureGenerationDialogState = new QCheckBox("Enable dynamic texture generation", this);

   QWidget* pOptionalMessagesWidget = new QWidget(this);
   QVBoxLayout* pOptionalMessagesLayout = new QVBoxLayout(pOptionalMessagesWidget);
   pOptionalMessagesLayout->setMargin(0);
   pOptionalMessagesLayout->setSpacing(5);
   pOptionalMessagesLayout->addWidget(mpEditMetadataDialogState);
   pOptionalMessagesLayout->addWidget(mpEnableDisplayAsDialogState);
   pOptionalMessagesLayout->addWidget(mpEnableTextureGenerationDialogState);
   pOptionalMessagesLayout->addStretch(10);
   LabeledSection* pOptionalMessagesSection = new LabeledSection(pOptionalMessagesWidget, 
      "Display Optional Messages", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pOptionalMessagesSection);
   pLayout->addStretch(10);

   Service<DesktopServices> pDesktop;

   mpEditMetadataDialogState->setChecked(!(pDesktop->getSuppressibleMsgDlgState(
      MetadataWidget::getEditWarningDialogId())));
   mpEnableDisplayAsDialogState->setChecked(!(pDesktop->getSuppressibleMsgDlgState(
      PropertiesRasterLayer::getDisplayAsWarningDialogId())));
   mpEnableTextureGenerationDialogState->setChecked(!(pDesktop->getSuppressibleMsgDlgState(
      PropertiesRasterLayer::getFilterWarningDialogId())));
}

void OptionsSuppressibleMsg::applyChanges()
{
   Service<DesktopServices> pDesktop;

   pDesktop->setSuppressibleMsgDlgState(MetadataWidget::getEditWarningDialogId(),
      !(mpEditMetadataDialogState->isChecked()));
   pDesktop->setSuppressibleMsgDlgState(PropertiesRasterLayer::getDisplayAsWarningDialogId(),
      !(mpEnableDisplayAsDialogState->isChecked()));
   pDesktop->setSuppressibleMsgDlgState(PropertiesRasterLayer::getFilterWarningDialogId(),
      !(mpEnableTextureGenerationDialogState->isChecked()));
}

OptionsSuppressibleMsg::~OptionsSuppressibleMsg() { }