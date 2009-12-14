/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFrame>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>

#include "ApplicationServices.h"
#include "BatchWizard.h"
#include "DataVariant.h"
#include "DataVariantEditor.h"
#include "DateTime.h"
#include "Filename.h"
#include "FileResource.h"
#include "LabeledSection.h"
#include "LabeledSectionGroup.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "WizardItem.h"
#include "WizardNode.h"
#include "WizardObject.h"
#include "WizardUtilities.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <fstream>
using namespace std;

string WizardUtilities::deriveBatchWizardFilename(const string& wizardFilename)
{
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(wizardFilename);
   string directory = pFilename->getPath();
   string title = pFilename->getTitle();
   return directory + "/" + title + ".batchwiz";
}

bool WizardUtilities::writeBatchWizard(const vector<BatchWizard*>& batchWizards, const string& batchFilename)
{
   if ((batchWizards.empty() == true) || (batchFilename.empty() == true))
   {
      return false;
   }

   Service<MessageLogMgr> pLogMgr;
   XMLWriter writer("batch", pLogMgr->getLog());

   for (vector<BatchWizard*>::const_iterator iter = batchWizards.begin(); iter != batchWizards.end(); ++iter)
   {
      BatchWizard* pBatchWizard = *iter;
      if (pBatchWizard != NULL)
      {
         bool success = pBatchWizard->toXml(&writer);
         if (!success)
         {
            return false;
         }
      }
   }

   FileResource batchFile(batchFilename.c_str(), "wb");
   if (batchFile.get() == NULL)
   {
      return false;
   }

   writer.writeToFile(batchFile);
   return true;
}

BatchWizard* WizardUtilities::createBatchWizardFromWizard(const string& wizardFilename)
{
   FactoryResource<WizardObject> pWizard(readWizard(wizardFilename));
   if (pWizard.get() != NULL)
   {
      return createBatchWizardFromWizard(pWizard.get(), wizardFilename);
   }
   return NULL;
}

BatchWizard* WizardUtilities::createBatchWizardFromWizard(WizardObject* pWizard, const string& wizardFilename)
{
   if (pWizard == NULL)
   {
      return NULL;
   }

   BatchWizard* pBatchWizard = new BatchWizard();
   if (pBatchWizard != NULL)
   {
      pBatchWizard->setWizardFilename(wizardFilename);

      // Values
      vector<WizardItem*> wizItems = pWizard->getItems();
      for (unsigned int i = 0; i < wizItems.size(); i++)
      {
         WizardItem* pItem = wizItems[i];
         if ((pItem != NULL) && (pItem->getType() != "Value"))
         {
            const string& itemName = pItem->getName();

            vector<WizardItem*> inputItems;
            pItem->getConnectedItems(true, inputItems);
            for (unsigned int j = 0; j < inputItems.size(); j++)
            {
               WizardItem* pInputItem = inputItems[j];
               if ((pInputItem != NULL) && (pInputItem->getType() == "Value"))
               {
                  vector<WizardNode*> wizNodes = pInputItem->getOutputNodes();
                  for (unsigned int n = 0; n < wizNodes.size(); n++)
                  {
                     string nodeName = wizNodes[n]->getName();
                     string nodeType = wizNodes[n]->getType();

                     void* pValue = wizNodes[n]->getValue();
                     if (pValue == NULL)
                     {
                        continue;
                     }

                     DataVariant value(nodeType, pValue);
                     pBatchWizard->setInputValue(itemName, nodeName, nodeType, value);
                  }
               }
            }
         }
      }
   }

   return pBatchWizard;
}

WizardObject* WizardUtilities::readWizard(const string& wizardFilename)
{
   if (wizardFilename.empty() == true)
   {
      return NULL;
   }

   FactoryResource<WizardObject> pWizard;

   XmlReader xml;
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(wizardFilename);
   bool bSuccess = false;

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDocument = xml.parse(pFilename.get());
   if (pDocument != NULL)
   {
      XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         unsigned int version = atoi(A(pRootElement->getAttribute(X("version"))));
         bSuccess = pWizard->fromXml(pRootElement, version);
      }
   }

   if (bSuccess == false)
   {
      return NULL;
   }

   return pWizard.release();
}

void WizardUtilities::runWizard(WizardObject* pWizard)
{
   if (pWizard == NULL)
   {
      return;
   }

   // Create a plug-in resource to ensure that the plug-in is unloaded
   string plugInName = "Wizard Executor";
   bool bBatch = pWizard->isBatch();

   ExecutableResource wizardExecutor(plugInName, string(), NULL, bBatch);
   VERIFYNRV(wizardExecutor->getPlugIn() != NULL);
   wizardExecutor->getInArgList().setPlugInArgValue("Wizard", pWizard);
   wizardExecutor->createProgressDialog(true);

   // Execute the wizard
   wizardExecutor->execute();
}

bool WizardUtilities::runBatchFiles(const vector<string>& batchWizardFiles, Progress* pProgress)
{
   Service<ApplicationServices> pApp;

   ExecutableResource batchWizardExecutor("Batch Wizard Executor", string(), pProgress, pApp->isBatch());
   if (pProgress == NULL)
   {
      batchWizardExecutor->createProgressDialog(true);
      pProgress = batchWizardExecutor->getProgress();
   }

   if (batchWizardExecutor->getPlugIn() == NULL)
   {
      if (pProgress != NULL)
      {
         string message = "Could not get the batch wizard executor plug-in.";
         pProgress->updateProgress(message, 0, ERRORS);
      }

      return false;
   }

   bool bOverallSuccess = true;

   unsigned int iCount = batchWizardFiles.size();

   // Launch the batch wizard executor for each xml file
   for (unsigned int i = 0; i < iCount; ++i)
   {
      string filename = batchWizardFiles[i];
      if (!filename.empty())
      {
         if (pProgress != NULL)
         {
            string message = "Processing batch file: " + filename;
            pProgress->updateProgress(message, static_cast<int>(i) * 100 / iCount, NORMAL);
         }

         FactoryResource<Filename> pFilename;
         VERIFY(pFilename.get() != NULL);
         pFilename->setFullPathAndName(filename);
         batchWizardExecutor->getInArgList().setPlugInArgValue("Filename", pFilename.get());
         bool bSuccess = batchWizardExecutor->execute();
         if (pProgress != NULL)
         {
            if (!bSuccess)
            {
               pProgress->updateProgress("Batch file failed: " + filename,
                  static_cast<int>(i + 1) * 100 / iCount, WARNING);
            }
            else
            {
               pProgress->updateProgress("Batch file complete: " + filename,
                  static_cast<int>(i + 1) * 100 / iCount, NORMAL);
            }
         }
         bOverallSuccess = bOverallSuccess && bSuccess;
      }
   }

   if (pProgress != NULL)
   {
      string message = "Processing batch files completed.";
      pProgress->updateProgress(message, 100, NORMAL);
   }

   return bOverallSuccess;
}

bool WizardUtilities::editItems(const vector<WizardItem*>& valueItems, QWidget* pParent)
{
   if (valueItems.empty() == true || Service<ApplicationServices>()->isBatch() == true)
   {
      return true;
   }

   QDialog input(pParent);

   // Edit widgets
   vector<DataVariantEditor*> editors;
   editors.reserve(valueItems.size());
   LabeledSectionGroup* pGroup = new LabeledSectionGroup(&input);
   for (size_t idx = 0; idx < valueItems.size(); ++idx)
   {
      DataVariantEditor* pEditor = new DataVariantEditor(&input);
      editors.push_back(pEditor);
      const vector<WizardNode*>& nodes = valueItems[idx]->getOutputNodes();
      VERIFY(nodes.size() == 1);
      WizardNode* pValNode = nodes.front();
      VERIFY(pValNode != NULL);
      DataVariant val(pValNode->getType(), pValNode->getValue());
      pEditor->setValue(val);

      LabeledSection* pSection = new LabeledSection(pEditor, QString::fromStdString(pValNode->getName()), &input);
      string connectionInfo = "<table width=100%>"
         "<tr><td width=20%><b>Name:</b></td><td>" + pValNode->getName() + "</td></tr>"
         "<tr><td width=20%><b>Type:</b></td><td>" + pValNode->getType() + "</td></tr>";

      const vector<WizardNode*>& connectedNodes = pValNode->getConnectedNodes();
      if (connectedNodes.empty() == true)
      {
         connectionInfo += "</table>";
      }
      else
      {
         connectionInfo += "<tr/><tr/><tr><td><b>Connections:</b></td></tr>"
            "<tr><td colspan=2><hr></td></tr></table>";
         for (vector<WizardNode*>::const_iterator iter = connectedNodes.begin(); iter != connectedNodes.end(); ++iter)
         {
            WizardNode* pCurrNode = *iter;
            VERIFY(pCurrNode != NULL);

            WizardItem* pCurrItem = pCurrNode->getItem();
            VERIFY(pCurrItem != NULL);

            connectionInfo +=
               "<table width=100%>"
               "<tr><td width=20%><b>Item:</b></td><td>" + pCurrItem->getName() + "</td></tr>"
               "<tr><td width=20%><b>Name:</b></td><td>" + pCurrNode->getName() + "</td></tr>"
               "<tr><td width=20%><b>Description:</b></td><td>" + pCurrNode->getDescription() + "</td></tr>"
               "<tr/></table>";
         }
      }

      pSection->setWhatsThis(QString::fromStdString(connectionInfo));

      DataVariantEditorDelegate valueDelegate = DataVariantEditor::getDelegate(pValNode->getType());
      if ((valueDelegate.getType() != DataVariantEditorDelegate::ENUMERATION) &&
         (valueDelegate.getType() != DataVariantEditorDelegate::VECTOR))
      {
         pEditor->setFixedHeight(pEditor->sizeHint().height());
         pGroup->addSection(pSection);
      }
      else
      {
         pGroup->addSection(pSection, 1000);
      }
   }

   pGroup->addStretch(1);

   // Horizontal line
   QFrame* pHLine = new QFrame(&input);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QDialogButtonBox* pButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, &input);

   // Layout
   QVBoxLayout* pLayout = new QVBoxLayout(&input);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(pGroup, 10);
   pLayout->addWidget(pHLine);
   pLayout->addWidget(pButtons);

   // Initialization
   input.setWindowTitle("Enter wizard values");
   input.resize(500, 500);

   // Connections
   VERIFYNR(input.connect(pButtons, SIGNAL(accepted()), &input, SLOT(accept())));
   VERIFYNR(input.connect(pButtons, SIGNAL(rejected()), &input, SLOT(reject())));

   // Execute the dialog
   if (input.exec() != QDialog::Accepted)
   {
      return false;
   }

   // Write the values to the nodes.
   for (size_t idx = 0; idx < valueItems.size(); ++idx)
   {
      DataVariantEditor* pEditor = editors[idx];
      const vector<WizardNode*>& nodes = valueItems[idx]->getOutputNodes();
      VERIFY(nodes.size() == 1);
      WizardNode* pValNode = nodes.front();
      VERIFY(pValNode != NULL);
      const DataVariant& val(pEditor->getValue());
      if (pValNode->getType() != val.getTypeName())
      {
         QMessageBox::warning(pParent, "Error", "The type cannot be changed.");
         return false;
      }

      pValNode->setValue(val.getPointerToValueAsVoid());
   }

   return true;
}
