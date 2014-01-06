/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "ConnectionParametersWidget.h"
#include "FileBrowser.h"
#include "Filename.h"
#include "ObjectResource.h"
#include "OptionsGeographicFeatures.h"
#include "ShapeFileImporter.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QRadioButton>
#include <QtGui/QStyleOptionButton>

#include <algorithm>

ConnectionParametersWidget::ConnectionParametersWidget(QWidget* pParent) :
   ModifierWidget(pParent),
   mShapelibConnection(true),
   mShpConnection(true),
   mSdeConnection(true)
{
   // Shape file
   mpShpButton = new QRadioButton("Shape file", this);

   mpShpWidget = new QWidget(this);
   QLabel* pFilenameLabel = new QLabel("Filename: ", mpShpWidget);
   mpFilenameEdit = new FileBrowser(mpShpWidget);
   mpUseArcCheck = new QCheckBox("Use ArcGIS", mpShpWidget);

   // Database
   mpSdeButton = new QRadioButton("ArcSDE Geodatabase", this);

   mpSdeWidget = new QWidget(this);
   QLabel* pUserLabel = new QLabel("Username: ", mpSdeWidget);
   QLabel* pPasswordLabel = new QLabel("Password: ", mpSdeWidget);
   QLabel* pDatabaseLabel = new QLabel("Database: ", mpSdeWidget);
   QLabel* pServerLabel = new QLabel("Server: ", mpSdeWidget);
   QLabel* pInstanceLabel = new QLabel("Instance: ", mpSdeWidget);
   QLabel* pVersionLabel = new QLabel("Version: ", mpSdeWidget);
   QLabel* pFeatureClassLabel = new QLabel("Feature class: ", mpSdeWidget);

   mpUserEdit = new QLineEdit(mpSdeWidget);
   mpPasswordEdit = new QLineEdit(mpSdeWidget);
   mpPasswordEdit->setEchoMode(QLineEdit::Password);
   mpDatabaseEdit = new QLineEdit(mpSdeWidget);
   mpServerEdit = new QLineEdit(mpSdeWidget);
   mpInstanceEdit = new QLineEdit(mpSdeWidget);
   mpVersionEdit = new QLineEdit(mpSdeWidget);
   mpFeatureClassEdit = new QLineEdit(mpSdeWidget);

   // Layout
   QGridLayout* pShpLayout = new QGridLayout(mpShpWidget);
   pShpLayout->setMargin(0);
   pShpLayout->setSpacing(5);
   pShpLayout->addWidget(pFilenameLabel, 0, 0);
   pShpLayout->addWidget(mpFilenameEdit, 0, 1);
   pShpLayout->addWidget(mpUseArcCheck, 1, 1);
   pShpLayout->setColumnStretch(1, 10);

   QGridLayout* pSdeLayout = new QGridLayout(mpSdeWidget);
   pSdeLayout->setMargin(0);
   pSdeLayout->setSpacing(5);
   pSdeLayout->addWidget(pUserLabel, 0, 0);
   pSdeLayout->addWidget(mpUserEdit, 0, 1);
   pSdeLayout->addWidget(pPasswordLabel, 1, 0);
   pSdeLayout->addWidget(mpPasswordEdit, 1, 1);
   pSdeLayout->addWidget(pDatabaseLabel, 2, 0);
   pSdeLayout->addWidget(mpDatabaseEdit, 2, 1);
   pSdeLayout->addWidget(pServerLabel, 3, 0);
   pSdeLayout->addWidget(mpServerEdit, 3, 1);
   pSdeLayout->addWidget(pInstanceLabel, 4, 0);
   pSdeLayout->addWidget(mpInstanceEdit, 4, 1);
   pSdeLayout->addWidget(pVersionLabel, 5, 0);
   pSdeLayout->addWidget(mpVersionEdit, 5, 1);
   pSdeLayout->addWidget(pFeatureClassLabel, 6, 0);
   pSdeLayout->addWidget(mpFeatureClassEdit, 6, 1);
   pSdeLayout->setColumnStretch(1, 10);

   QStyleOptionButton option;
   option.initFrom(mpShpButton);
   int radioWidth = style()->subElementRect(QStyle::SE_RadioButtonIndicator, &option).width();

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->setColumnMinimumWidth(0, radioWidth);
   pLayout->addWidget(mpShpButton, 0, 0, 1, 2);
   pLayout->addWidget(mpShpWidget, 1, 1);
   pLayout->addWidget(mpSdeButton, 2, 0, 1, 2);
   pLayout->addWidget(mpSdeWidget, 3, 1);
   pLayout->setRowStretch(4, 10);
   pLayout->setColumnStretch(1, 10);

   // Initialization
   QString directory;

   const Filename* pWorkingDir = NULL;
   if (ConfigurationSettings::hasSettingPluginWorkingDirectory(ShapeFileImporter::PLUGIN_SUBTYPE))
   {
      pWorkingDir = ConfigurationSettings::getSettingPluginWorkingDirectory(ShapeFileImporter::PLUGIN_SUBTYPE);
   }
   else
   {
      pWorkingDir = ConfigurationSettings::getSettingImportPath();
   }

   if (pWorkingDir != NULL)
   {
      directory = QString::fromStdString(pWorkingDir->getFullPathAndName());
   }

   mpFilenameEdit->setBrowseCaption("Select Shape File");
   mpFilenameEdit->setBrowseDirectory(directory);
   mpFilenameEdit->setBrowseFileFilters("Shape Files (*.shp);;All Files (*)");
   mpUseArcCheck->setChecked(OptionsGeographicFeatures::getSettingUseArcAsDefaultConnection());

   mpShpButton->setChecked(true);
   updateWidgets();

   // Connections
   VERIFYNR(connect(mpShpButton, SIGNAL(toggled(bool)), this, SLOT(updateWidgets())));
   VERIFYNR(connect(mpFilenameEdit, SIGNAL(filenameChanged(const QString&)), this,
      SLOT(updateWorkingDirectory(const QString&))));
   VERIFYNR(connect(mpSdeButton, SIGNAL(toggled(bool)), this, SLOT(updateWidgets())));

   VERIFYNR(attachSignal(mpShpButton, SIGNAL(toggled(bool))));
   VERIFYNR(attachSignal(mpFilenameEdit, SIGNAL(filenameChanged(const QString&))));
   VERIFYNR(attachSignal(mpUseArcCheck, SIGNAL(toggled(bool))));
   VERIFYNR(attachSignal(mpSdeButton, SIGNAL(toggled(bool))));
   VERIFYNR(attachSignal(mpUserEdit, SIGNAL(textEdited(const QString&))));
   VERIFYNR(attachSignal(mpPasswordEdit, SIGNAL(textEdited(const QString&))));
   VERIFYNR(attachSignal(mpDatabaseEdit, SIGNAL(textEdited(const QString&))));
   VERIFYNR(attachSignal(mpServerEdit, SIGNAL(textEdited(const QString&))));
   VERIFYNR(attachSignal(mpInstanceEdit, SIGNAL(textEdited(const QString&))));
   VERIFYNR(attachSignal(mpVersionEdit, SIGNAL(textEdited(const QString&))));
   VERIFYNR(attachSignal(mpFeatureClassEdit, SIGNAL(textEdited(const QString&))));
}

ConnectionParametersWidget::~ConnectionParametersWidget()
{}

ArcProxyLib::ConnectionParameters ConnectionParametersWidget::getConnectionParameters() const
{
   ArcProxyLib::ConnectionParameters connection;
   if (mpSdeButton->isChecked())
   {
      connection.setUser(mpUserEdit->text().toStdString());
      connection.setPassword(mpPasswordEdit->text().toStdString());
      connection.setDatabase(mpDatabaseEdit->text().toStdString());
      connection.setServer(mpServerEdit->text().toStdString());
      connection.setInstance(mpInstanceEdit->text().toStdString());
      connection.setVersion(mpVersionEdit->text().toStdString());
      connection.setFeatureClass(mpFeatureClassEdit->text().toStdString());
      connection.setConnectionType(ArcProxyLib::SDE_CONNECTION);
   }
   else
   {
      QFileInfo fileInfo(mpFilenameEdit->getFilename());
      connection.setDatabase(fileInfo.absolutePath().toStdString());
      connection.setFeatureClass(fileInfo.fileName().toStdString());

      if (mpUseArcCheck->isChecked())
      {
         connection.setConnectionType(ArcProxyLib::SHP_CONNECTION);
      }
      else
      {
         connection.setConnectionType(ArcProxyLib::SHAPELIB_CONNECTION);
      }
   }

   return connection;
}

void ConnectionParametersWidget::setConnectionParameters(const ArcProxyLib::ConnectionParameters& connection)
{
   if (connection.getConnectionType() == ArcProxyLib::SDE_CONNECTION)
   {
      mpUserEdit->setText(QString::fromStdString(connection.getUser()));
      mpPasswordEdit->setText(QString::fromStdString(connection.getPassword()));
      mpDatabaseEdit->setText(QString::fromStdString(connection.getDatabase()));
      mpServerEdit->setText(QString::fromStdString(connection.getServer()));
      mpInstanceEdit->setText(QString::fromStdString(connection.getInstance()));
      mpVersionEdit->setText(QString::fromStdString(connection.getVersion()));
      mpFeatureClassEdit->setText(QString::fromStdString(connection.getFeatureClass()));
      mpSdeButton->setChecked(true);
   }
   else
   {
      QString filename;
      QString database = QString::fromStdString(connection.getDatabase());
      if (!database.isEmpty())
      {
         QDir dir(QString::fromStdString(connection.getDatabase()));
         filename = dir.absoluteFilePath(QString::fromStdString(connection.getFeatureClass()));
      }

      mpFilenameEdit->setFilename(filename);
      mpShpButton->setChecked(true);

      if (mpUseArcCheck->isEnabled() == true)
      {
         mpUseArcCheck->setChecked(connection.getConnectionType() == ArcProxyLib::SHP_CONNECTION);
      }
   }
}

void ConnectionParametersWidget::setAvailableConnectionTypes(const std::vector<ArcProxyLib::ConnectionType>& types)
{
   mShapelibConnection = (std::find(types.begin(), types.end(), ArcProxyLib::SHAPELIB_CONNECTION) != types.end());
   mShpConnection = (std::find(types.begin(), types.end(), ArcProxyLib::SHP_CONNECTION) != types.end());
   mSdeConnection = (std::find(types.begin(), types.end(), ArcProxyLib::SDE_CONNECTION) != types.end());

   if (mShpConnection == false)
   {
      if (mShapelibConnection == false)
      {
         mpFilenameEdit->setFilename(QString());

         if (mSdeConnection == true)
         {
            mpSdeButton->setChecked(true);
         }
      }

      mpUseArcCheck->setChecked(false);
   }

   if (mSdeConnection == false)
   {
      if ((mShapelibConnection == true) || (mShpConnection == true))
      {
         mpShpButton->setChecked(true);
      }

      mpUserEdit->clear();
      mpPasswordEdit->clear();
      mpDatabaseEdit->clear();
      mpServerEdit->clear();
      mpInstanceEdit->clear();
      mpVersionEdit->clear();
      mpFeatureClassEdit->clear();
   }

   mpShpButton->setEnabled(mShapelibConnection || mShpConnection);
   mpSdeButton->setEnabled(mSdeConnection);

   updateWidgets();
}

void ConnectionParametersWidget::updateWorkingDirectory(const QString& filename)
{
   if (filename.isEmpty() == true)
   {
      return;
   }

   QFileInfo fileInfo(filename);
   QDir newDirectory(fileInfo.absoluteDir());
   if (newDirectory.exists() == true)
   {
      FactoryResource<Filename> pNewWorkingDir;
      pNewWorkingDir->setFullPathAndName(newDirectory.path().toStdString());

      Service<ConfigurationSettings> pSettings;
      pSettings->setTemporarySetting(ConfigurationSettings::getSettingPluginWorkingDirectoryKey(
         ShapeFileImporter::PLUGIN_SUBTYPE), *pNewWorkingDir.get());
   }
}

void ConnectionParametersWidget::updateWidgets()
{
   mpShpWidget->setEnabled(mpShpButton->isEnabled() && mpShpButton->isChecked());
   mpUseArcCheck->setEnabled(mpShpWidget->isEnabled() && mShpConnection);
   mpSdeWidget->setEnabled(mpSdeButton->isEnabled() && mpSdeButton->isChecked());
}
