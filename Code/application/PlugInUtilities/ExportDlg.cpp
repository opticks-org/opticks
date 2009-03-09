/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtGui/QListView>
#include <QtGui/QMessageBox>

#include "AppVersion.h"
#include "ConfigurationSettings.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "ExportDlg.h"
#include "ExportOptionsDlg.h"
#include "FileDescriptor.h"

#include <string>
using namespace std;

ExportDlg::ExportDlg(ExporterResource& pExporter,
                     const vector<PlugInDescriptor*>& availablePlugins,
                     QWidget* pParent) :
   FilePlugInDlg(availablePlugins, ConfigurationSettings::getSettingPluginWorkingDirectoryKey("Exporter"), pParent),
   mpExporter(pExporter)
{
   // Initializtion
   setWindowTitle("Export");
   setFileMode(QFileDialog::AnyFile);
   setAcceptMode(QFileDialog::AcceptSave);
   setConfirmOverwrite(true);
   setLabelText(QFileDialog::LookIn, "Save in:");
   setPlugInLabel("Exporter");
   enableOptions(true);

   // Set the initial directory
   string directory;
   const Filename* pWorkingDir = NULL;
   Service<ConfigurationSettings> pSettings;
   pWorkingDir = pSettings->getSetting(ConfigurationSettings::getSettingPluginWorkingDirectoryKey("Exporter")).getPointerToValue<Filename>();
   if (pWorkingDir == NULL)
   {
      pWorkingDir = ConfigurationSettings::getSettingExportPath();
   }
   if (pWorkingDir != NULL)
   {
      directory = pWorkingDir->getFullPathAndName();
   }

   if(!directory.empty())
   {
      setDirectory(QString::fromStdString(directory));
   }

   if (isDefaultPlugIn())
   {
      setSelectedPlugIn("Ice Exporter");
   }
   updateFromExporter(getSelectedPlugIn());

   // Set the initially selected file
   QString strFilename;

   SessionItem* pSessionItem = mpExporter->getItem();
   if (pSessionItem != NULL)
   {
      DataElement* pDataElement = dynamic_cast<DataElement*>(pSessionItem);
      if (pDataElement != NULL)
      {
         strFilename = QString::fromStdString(pDataElement->getFilename());
      }

      if (strFilename.isEmpty() == true)
      {
         strFilename = QString::fromStdString(pSessionItem->getName());
      }

      strFilename = updateExtension(strFilename);
      selectFile(strFilename);

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Do not clear the initial list view selection " \
   "when it is removed from the internal QFileDialog list of items that are not filtered! (Qt 4.3.1) (dsulgrov)")
      QListView* pListView = findChild<QListView*>("listView");
      if (pListView != NULL)
      {
         pListView->clearSelection();
      }
   }

   updateFromFile(strFilename);

   // Connections
   connect(this, SIGNAL(plugInSelected(const QString&)), this, SLOT(updateFromExporter(const QString&)));
   connect(this, SIGNAL(optionsClicked()), this, SLOT(invokeOptionsDialog()));
}

ExportDlg::~ExportDlg()
{
}

ExporterResource& ExportDlg::getExporterResource() const
{
   return mpExporter;
}

void ExportDlg::accept()
{
   // Check for a valid plug-in
   QString strPlugIn = getSelectedPlugIn();
   if (strPlugIn.isEmpty() == true)
   {
      QMessageBox::critical(this, APP_NAME, "The selected exporter is invalid!");
      return;
   }

   // Validate with the exporter
   bool validating = true;
   do
   {
      Service<DesktopServices> pDesktop;
      string errorMessage;

      // Update the file descriptor with the selected file
      QString strFilename = getExportFile();
      if (QFileInfo(strFilename).isDir())
      {
         break;
      }
      updateFromFile(updateExtension(strFilename, true, false));
      switch (mpExporter->validate(errorMessage))
      {
      case VALIDATE_SUCCESS:
         validating = false;
         break;
      case VALIDATE_FAILURE:
         if (errorMessage.empty())
         {
            errorMessage = "Unable to validate inputs.";
         }
         pDesktop->showMessageBox("Validation Error", errorMessage, "&Cancel");
         return;
      case VALIDATE_INFO:
         // Exporter's returning VALIDATE_INFO must provide a value
         // for error message...not providing a value is a programmer error
         VERIFYNRV(!errorMessage.empty());
         switch (pDesktop->showMessageBox("Validation Information", errorMessage,
            "&Ok", "&Options", "&Cancel", 0, 2))
         {
         case 0: // Ok
            validating = false;
            break;
         case 1: // Options
            if (invokeOptionsDialog())
            {
               break;
            }
            // fall through
         case 2: // Cancel
            return;
         }
         break;
      case VALIDATE_INPUT_REQUIRED:
         if (!invokeOptionsDialog())
         {
            return;
         }
         break;
      default:
         VERIFYNRV_MSG(false, "An invalid verification type was returned.");
      }
   }
   while (validating);

   FilePlugInDlg::accept();
}

void ExportDlg::hideEvent(QHideEvent* pEvent)
{
   // Set the selected directory to the current directory
   QDir dir = directory();
   QDir::setCurrent(dir.absolutePath());

   FilePlugInDlg::hideEvent(pEvent);
}

QString ExportDlg::getExportFile() const
{
   QStringList filenames = selectedFiles();
   if (filenames.empty() == false)
   {
      return filenames.front();
   }

   return QString();
}

void ExportDlg::updateFromFile(const QString& strFilename)
{
   FileDescriptor* pFileDescriptor = mpExporter->getFileDescriptor();
   if (pFileDescriptor != NULL)
   {
      string filename;
      if (strFilename.isEmpty() == false)
      {
         QFileInfo fileInfo(strFilename);
         if (fileInfo.isDir() == false)
         {
            filename = fileInfo.absoluteFilePath().toStdString();
         }
      }

      pFileDescriptor->setFilename(filename);
   }
}

QString ExportDlg::updateExtension(const QString& strFilename,
                                   bool preserveFullPath,
                                   bool alwaysReplaceExtension)
{
   if (strFilename.isEmpty())
   {
      return strFilename;
   }

   QFileInfo fileInfo(strFilename);
   if (fileInfo.isDir())
   {
      return "";
   }
   QString orgFilename = (preserveFullPath ? strFilename : fileInfo.fileName());

   QString strCurrentFilter = selectedFilter();
   int iOpenParen = strCurrentFilter.indexOf("(");
   int iCloseParen = strCurrentFilter.indexOf(")");

   QString strFilterExtension;
   if (iOpenParen != -1 && iCloseParen != -1)
   {
      strFilterExtension = strCurrentFilter.mid(iOpenParen + 1, iCloseParen - iOpenParen - 1);
      int filterStart = strFilterExtension.indexOf(".");
      int filterEnd = strFilterExtension.indexOf(" ");
      if (filterEnd == -1)
      {
         filterEnd = strFilterExtension.length();
      }
      if (filterStart != -1)
      {
         strFilterExtension = strFilterExtension.mid(filterStart, filterEnd - filterStart);
      }
      strFilterExtension = strFilterExtension.trimmed();
   }

   if (strFilterExtension.isEmpty() || strFilterExtension == "*")
   {
      return orgFilename;
   }
   QString newFilename;
   QString existingExtension = fileInfo.completeSuffix();
   QString orgFilenameWithoutExtension = fileInfo.baseName();
   if (alwaysReplaceExtension || existingExtension.isEmpty())
   {
      newFilename = orgFilenameWithoutExtension + strFilterExtension;
   }
   else
   {
      newFilename = orgFilenameWithoutExtension + "." + existingExtension;
   }
   return (preserveFullPath ? fileInfo.path() + "/" + newFilename : newFilename);
}

void ExportDlg::updateFromExporter(const QString& strExporter)
{
   // Update the exporter
   if (strExporter.isEmpty() == false)
   {
      selectFile(updateExtension(getExportFile()));
      updateFromFile(getExportFile());
      string exporter = strExporter.toStdString();
      mpExporter->setPlugIn(exporter);
   }
   else
   {
      mpExporter->setPlugIn(NULL);
   }
}

bool ExportDlg::invokeOptionsDialog()
{
   // Update the file descriptor with the selected file
   QString strFilename = getExportFile();
   updateFromFile(strFilename);

   // Display the dialog
   ExportOptionsDlg optionsDlg(mpExporter, this);
   if (optionsDlg.exec() == QDialog::Accepted)
   {
      return true;
   }

   return false;
}
