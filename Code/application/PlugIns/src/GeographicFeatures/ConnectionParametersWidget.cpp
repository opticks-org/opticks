/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "ConfigurationSettings.h"
#include "ConnectionParametersWidget.h"
#include "AppVerify.h"
#include "Filename.h"
#include "IconImages.h"
#include "ShapeFileImporter.h"

#include <QtGui/QCheckBox>
#include <QtGui/QFileDialog>
#include <QtGui/QBitmap>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <algorithm>
#include <boost/bind.hpp>

ConnectionParametersWidget::ConnectionParametersWidget(QWidget *pParent) : QWidget(pParent), mModified(false)
{
   QLabel *pFilenameLabel = new QLabel("Filename: ", this);
   QLabel *pUserLabel = new QLabel("Username: ", this);
   QLabel *pPasswordLabel = new QLabel("Password: ", this);
   QLabel *pDatabaseLabel = new QLabel("Database: ", this);
   QLabel *pServerLabel = new QLabel("Server: ", this);
   QLabel *pInstanceLabel = new QLabel("Instance: ", this);
   QLabel *pVersionLabel = new QLabel("Version: ", this);
   QLabel *pFeatureClassLabel = new QLabel("Feature class: ", this);

   mFileWidgets.push_back(pFilenameLabel);
   mDatabaseWidgets.push_back(pUserLabel);
   mDatabaseWidgets.push_back(pPasswordLabel);
   mDatabaseWidgets.push_back(pDatabaseLabel);
   mDatabaseWidgets.push_back(pServerLabel);
   mDatabaseWidgets.push_back(pInstanceLabel);
   mDatabaseWidgets.push_back(pVersionLabel);
   mDatabaseWidgets.push_back(pFeatureClassLabel);

   mpShpButton = new QRadioButton("Shape file", this);
   mpSdeButton = new QRadioButton("ArcSDE Geodatabase", this);

   mpUseArcCheck = new QCheckBox("Use ArcGIS", this);
   // mpUseArcCheck gets special treatment in updateWidgets, don't need to add it to a vector

   QPixmap openPix(IconImages::OpenIcon);
   openPix.setMask(openPix.createHeuristicMask());

   mpSelectFileButton = new QPushButton(QIcon(openPix), QString(), this);
   mFileWidgets.push_back(mpSelectFileButton);

   mpFilenameEdit = new QLineEdit(this);
   mpUserEdit = new QLineEdit(this);
   mpPasswordEdit = new QLineEdit(this);
   mpDatabaseEdit = new QLineEdit(this);
   mpServerEdit = new QLineEdit(this);
   mpInstanceEdit = new QLineEdit(this);
   mpVersionEdit = new QLineEdit(this);
   mpFeatureClassEdit = new QLineEdit(this);

   mFileWidgets.push_back(mpFilenameEdit);
   mDatabaseWidgets.push_back(mpUserEdit);
   mDatabaseWidgets.push_back(mpPasswordEdit);
   mDatabaseWidgets.push_back(mpDatabaseEdit);
   mDatabaseWidgets.push_back(mpServerEdit);
   mDatabaseWidgets.push_back(mpInstanceEdit);
   mDatabaseWidgets.push_back(mpVersionEdit);
   mDatabaseWidgets.push_back(mpFeatureClassEdit);

   QGridLayout *pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->setColumnMinimumWidth(0, 15);
   pLayout->addWidget(mpShpButton, 0, 0, 1, 4);
   pLayout->addWidget(pFilenameLabel, 1, 1);
   pLayout->addWidget(mpFilenameEdit, 1, 2);
   pLayout->addWidget(mpSelectFileButton, 1, 3);
   pLayout->addWidget(mpUseArcCheck, 2, 2, 1, 2);
   pLayout->addWidget(mpSdeButton, 3, 0, 1, 4);
   pLayout->addWidget(pUserLabel, 4, 1);
   pLayout->addWidget(mpUserEdit, 4, 2, 1, 2);
   pLayout->addWidget(pPasswordLabel, 5, 1);
   pLayout->addWidget(mpPasswordEdit, 5, 2, 1, 2);
   pLayout->addWidget(pDatabaseLabel, 6, 1);
   pLayout->addWidget(mpDatabaseEdit, 6, 2, 1, 2);
   pLayout->addWidget(pServerLabel, 7, 1);
   pLayout->addWidget(mpServerEdit, 7, 2, 1, 2);
   pLayout->addWidget(pInstanceLabel, 8, 1);
   pLayout->addWidget(mpInstanceEdit, 8, 2, 1, 2);
   pLayout->addWidget(pVersionLabel, 9, 1);
   pLayout->addWidget(mpVersionEdit, 9, 2, 1, 2);
   pLayout->addWidget(pFeatureClassLabel, 10, 1);
   pLayout->addWidget(mpFeatureClassEdit, 10, 2, 1, 2);

   pLayout->setRowStretch(11, 10);

   VERIFYNR(connect(mpFilenameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpUserEdit, SIGNAL(textEdited(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpPasswordEdit, SIGNAL(textEdited(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpDatabaseEdit, SIGNAL(textEdited(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpServerEdit, SIGNAL(textEdited(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpInstanceEdit, SIGNAL(textEdited(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpVersionEdit, SIGNAL(textEdited(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpFeatureClassEdit, SIGNAL(textEdited(const QString&)), this, SLOT(setModified())));
   VERIFYNR(connect(mpUseArcCheck, SIGNAL(toggled(bool)), this, SLOT(setModified())));
   VERIFYNR(connect(mpShpButton, SIGNAL(toggled(bool)), this, SLOT(setModified())));
   VERIFYNR(connect(mpSdeButton, SIGNAL(toggled(bool)), this, SLOT(setModified())));

   VERIFYNR(connect(mpSelectFileButton, SIGNAL(clicked(bool)), this, SLOT(selectFileClicked())));
   VERIFYNR(connect(mpShpButton, SIGNAL(toggled(bool)), this, SLOT(updateWidgets())));
   VERIFYNR(connect(mpSdeButton, SIGNAL(toggled(bool)), this, SLOT(updateWidgets())));
}

ConnectionParametersWidget::~ConnectionParametersWidget()
{
}

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
      QFileInfo fileInfo(mpFilenameEdit->text());
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

void ConnectionParametersWidget::setConnectionParameters(const ArcProxyLib::ConnectionParameters &connection)
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

      mpFilenameEdit->setText(filename);

      mpShpButton->setChecked(true);
      mpUseArcCheck->setChecked(connection.getConnectionType() == ArcProxyLib::SHP_CONNECTION);
   }
}

bool ConnectionParametersWidget::isModified() const
{
   return mModified;
}

void ConnectionParametersWidget::setModified(bool modified)
{
   mModified = modified;
}

void ConnectionParametersWidget::updateWidgets()
{
   bool isFile = false;

   if (mpShpButton->isChecked())
   {
      isFile = true;
   }

   std::for_each(mFileWidgets.begin(), mFileWidgets.end(),
      boost::bind(&QWidget::setEnabled, _1, isFile));
   std::for_each(mDatabaseWidgets.begin(), mDatabaseWidgets.end(), 
      boost::bind(&QWidget::setDisabled, _1, isFile));

   mpUseArcCheck->setEnabled(isFile && mpSdeButton->isEnabled());

   setModified(true);
}

void ConnectionParametersWidget::selectFileClicked()
{
   QString directory = mpFilenameEdit->text();
   if (directory.isEmpty())
   {
      const Filename* pWorkingDir = NULL;
      if (ConfigurationSettings::hasSettingPluginWorkingDirectory(ShapeFileImporter::PLUGIN_SUBTYPE))
      {
         pWorkingDir = ConfigurationSettings::getSettingPluginWorkingDirectory(ShapeFileImporter::PLUGIN_SUBTYPE);
      }
      else
      {
         pWorkingDir = ConfigurationSettings::getSettingImportExportPath();
      }
      if (pWorkingDir != NULL)
      {
         directory = QString::fromStdString(pWorkingDir->getFullPathAndName());
      }
   }
   QString filename = QFileDialog::getOpenFileName(this, "Select shape file", directory, "Shape files(*.shp)");
   if (!filename.isEmpty())
   {
      mpFilenameEdit->setText(filename);

      QFileInfo fileInfo(filename);
      QDir newDirectory(fileInfo.absoluteDir());

      newDirectory.path();
      FactoryResource<Filename> pNewWorkingDir;
      pNewWorkingDir->setFullPathAndName(newDirectory.path().toStdString());

      Service<ConfigurationSettings> pSettings;
      pSettings->setSessionSetting(ConfigurationSettings::getSettingPluginWorkingDirectoryKey(ShapeFileImporter::PLUGIN_SUBTYPE), *pNewWorkingDir.get());
   }
}

void ConnectionParametersWidget::setAvailableConnectionTypes(
   const std::vector<ArcProxyLib::ConnectionType> &types)
{
   mpUseArcCheck->setEnabled(false);
   mpShpButton->setEnabled(false);
   mpSdeButton->setEnabled(false);

   for (std::vector<ArcProxyLib::ConnectionType>::const_iterator iter = types.begin();
      iter != types.end(); ++iter)
   {
      switch (*iter)
      {
      case ArcProxyLib::SHAPELIB_CONNECTION:
         mpShpButton->setEnabled(true);
         break;
      case ArcProxyLib::SHP_CONNECTION:
         mpShpButton->setEnabled(true);
         mpUseArcCheck->setEnabled(true);
         break;
      case ArcProxyLib::SDE_CONNECTION:
         mpSdeButton->setEnabled(true);
         break;
      default:
         break;
      }
   }
}
