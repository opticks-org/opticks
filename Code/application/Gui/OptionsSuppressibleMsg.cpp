/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariantEditor.h"
#include "DesktopServices.h"
#include "LabeledSection.h"
#include "MetadataWidget.h"
#include "OptionsSuppressibleMsg.h"
#include "PropertiesRasterLayer.h"

#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

OptionsSuppressibleMsg::OptionsSuppressibleMsg() :
   QWidget(NULL)
{
   // To add a new Suppressible option to the list, add an entry to the "entries" vector with a description and an id.
   std::vector<std::pair<QString, std::string> > entries;
   entries.push_back(std::make_pair("Edit metadata", MetadataWidget::getEditWarningDialogId()));
   entries.push_back(std::make_pair("Warn when \"Display As\" menu commands fail",
      PropertiesRasterLayer::getDisplayAsWarningDialogId()));
   entries.push_back(std::make_pair("Enable dynamic texture generation",
      PropertiesRasterLayer::getFilterWarningDialogId()));
   entries.push_back(std::make_pair("Warn when entering comma-spaces within vector<string> entries",
      DataVariantEditor::getVectorStringEditWarningDialogId()));

   // Everything from this point is generic.
   QWidget* pOptionalMessagesWidget = new QWidget(this);
   QVBoxLayout* pOptionalMessagesLayout = new QVBoxLayout(pOptionalMessagesWidget);
   pOptionalMessagesLayout->setMargin(0);
   pOptionalMessagesLayout->setSpacing(5);
   for (std::vector<std::pair<QString, std::string> >::const_iterator iter = entries.begin();
      iter != entries.end();
      ++iter)
   {
      QCheckBox* pCheckBox = new QCheckBox(iter->first, this);
      pCheckBox->setChecked(!(Service<DesktopServices>()->getSuppressibleMsgDlgState(iter->second)));
      pOptionalMessagesLayout->addWidget(pCheckBox);
      mCheckBoxes.push_back(std::make_pair(pCheckBox, iter->second));
   }
   pOptionalMessagesLayout->addStretch(10);
   LabeledSection* pOptionalMessagesSection = new LabeledSection(pOptionalMessagesWidget,
      "Display Optional Messages", this);

   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pOptionalMessagesSection);
   pLayout->addStretch(10);
}

OptionsSuppressibleMsg::~OptionsSuppressibleMsg()
{}

void OptionsSuppressibleMsg::applyChanges()
{
   for (std::vector<std::pair<QCheckBox*, std::string> >::const_iterator iter = mCheckBoxes.begin();
      iter != mCheckBoxes.end();
      ++iter)
   {
      Service<DesktopServices>()->setSuppressibleMsgDlgState(iter->second, !iter->first->isChecked());
   }
}
