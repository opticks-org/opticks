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
#include "IgmGeoreference.h"
#include "IgmGui.h"

#include <QtGui/QGridLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QStyleOptionButton>

IgmGui::IgmGui(QWidget* pParent) :
   QWidget(pParent),
   mpDescriptor(NULL)
{
   mpUseExisting = new QRadioButton("Use existing IGM", this);
   mpLoad = new QRadioButton("Load an IGM file:", this);
   mpFilename = new FileBrowser(this);
   mpFilename->setToolTip("An IGM file is an Image Geometry file used for georeferencing and georectification.");
   mpFilename->setBrowseCaption("Select IGM File");
   mpFilename->setBrowseFileFilters("IGM Files (*.igm *.hdr)");
   mpFilename->setBrowseExistingFile(true);

   // Layout
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

   // Initialization
   mpUseExisting->setEnabled(false);
   mpLoad->setChecked(true);

   // Connections
   VERIFYNR(connect(mpUseExisting, SIGNAL(toggled(bool)), this, SLOT(setExistingElement(bool))));
   VERIFYNR(connect(mpLoad, SIGNAL(toggled(bool)), mpFilename, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpFilename, SIGNAL(filenameChanged(const QString&)), this, SLOT(setIgmFilename(const QString&))));

   mpDescriptor.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &IgmGui::georeferenceDescriptorModified));
}

IgmGui::~IgmGui()
{}

void IgmGui::setGeoreferenceData(GeoreferenceDescriptor* pDescriptor, bool enableExistingElement,
                                 bool enableIgmFilename)
{
   if ((pDescriptor != mpDescriptor.get()) || (mpUseExisting->isEnabled() != enableExistingElement) ||
      (mpLoad->isEnabled() != enableIgmFilename))
   {
      mpDescriptor.reset(pDescriptor);

      // Enable/disable the radio buttons based on their availability
      mpUseExisting->setEnabled(enableExistingElement);
      mpLoad->setEnabled(enableIgmFilename);

      // Update the georeference descriptor attribute to use the existing element based on the enabled radio buttons
      if (mpDescriptor.get() != NULL)
      {
         bool useExistingElement = dv_cast<bool>(mpDescriptor->getAttributeByPath(USE_EXISTING_ELEMENT), false);
         if ((useExistingElement == true) && (enableExistingElement == false))
         {
            mpDescriptor->setAttributeByPath(USE_EXISTING_ELEMENT, false);
         }
         else if ((useExistingElement == false) && (enableExistingElement == true))
         {
            // If the existing element is available and the IGM filename is not specified,
            // set the default to use the existing element
            std::string filename = dv_cast<std::string>(mpDescriptor->getAttributeByPath(IGM_FILENAME), std::string());
            if (filename.empty() == true)
            {
               mpDescriptor->setAttributeByPath(USE_EXISTING_ELEMENT, true);
            }
         }
      }

      // Update the widgets based on the georeference parameters
      updateFromGeoreferenceDescriptor();
   }
}

GeoreferenceDescriptor* IgmGui::getGeoreferenceDescriptor()
{
   return mpDescriptor.get();
}

const GeoreferenceDescriptor* IgmGui::getGeoreferenceDescriptor() const
{
   return mpDescriptor.get();
}

void IgmGui::georeferenceDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   updateFromGeoreferenceDescriptor();
}

void IgmGui::updateFromGeoreferenceDescriptor()
{
   if (mpDescriptor.get() != NULL)
   {
      // Need to block signals when updating the widgets because DynamicObject::setAttributeByPath() does
      // not check if the attribute value is modified before setting the value and notifying the signal

      bool useExistingElement = dv_cast<bool>(mpDescriptor->getAttributeByPath(USE_EXISTING_ELEMENT), false);
      mpUseExisting->blockSignals(true);
      mpUseExisting->setChecked(useExistingElement);
      mpUseExisting->blockSignals(false);

      std::string filename = dv_cast<std::string>(mpDescriptor->getAttributeByPath(IGM_FILENAME), std::string());
      mpFilename->blockSignals(true);
      mpFilename->setFilename(QString::fromStdString(filename));
      mpFilename->blockSignals(false);
   }
}

void IgmGui::setExistingElement(bool useExistingElement)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath(USE_EXISTING_ELEMENT, useExistingElement);
   }
}

void IgmGui::setIgmFilename(const QString& filename)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath(IGM_FILENAME, filename.toStdString());
   }
}
