/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QCompleter>
#include <QtGui/QDirModel>
#include <QtGui/QFileDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>

#include "AppAssert.h"
#include "AppVerify.h"
#include "FileBrowser.h"
#include "Filename.h"
#include "IconImages.h"

FileBrowser::FileBrowser(QWidget* pParent) :
   QWidget(pParent),
   mExistingFile(true)
{
   // Filename edit
   mpFileEdit = new QLineEdit(this);
   QCompleter *pCompleter = new QCompleter(this);
   pCompleter->setModel(new QDirModel(pCompleter));
   mpFileEdit->setCompleter(pCompleter);

   // Browse button
   QPixmap pixOpen(IconImages::OpenIcon);
   pixOpen.setMask(pixOpen.createHeuristicMask());
   QIcon icnBrowse(pixOpen);

   QPushButton* pBrowseButton = new QPushButton(icnBrowse, QString(), this);
   pBrowseButton->setFixedWidth(27);

   // Layout
   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpFileEdit, 10);
   pLayout->addWidget(pBrowseButton);

   // Connections
   VERIFYNR(connect(mpFileEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(filenameChanged(const QString&))));
   VERIFYNR(connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(browse())));
}

FileBrowser::~FileBrowser()
{
}

void FileBrowser::setFilename(const QString& filename)
{
   if (filename != getFilename())
   {
      mpFileEdit->setText(filename);
   }
}

void FileBrowser::setFilename(const Filename& filename)
{
   QString filenameText = QString::fromStdString(filename.getFullPathAndName());
   setFilename(filenameText);
}

QString FileBrowser::getFilename() const
{
   return mpFileEdit->text();
}

void FileBrowser::setBrowseCaption(const QString& caption)
{
   mBrowseCaption = caption;
}

QString FileBrowser::getBrowseCaption() const
{
   return mBrowseCaption;
}

void FileBrowser::setBrowseDirectory(const QString& directory)
{
   mBrowseDirectory = directory;
}

QString FileBrowser::getBrowseDirectory() const
{
   return mBrowseDirectory;
}

void FileBrowser::setBrowseFileFilters(const QString& filters)
{
   mBrowseFilters = filters;
}

QString FileBrowser::getBrowseFileFilters() const
{
   return mBrowseFilters;
}

void FileBrowser::setBrowseExistingFile(bool bExistingFile)
{
   mExistingFile = bExistingFile;
}

bool FileBrowser::isBrowseExistingFile() const
{
   return mExistingFile;
}

void FileBrowser::browse()
{
   // Get the initial browse directory
   QString browseDirectory = getFilename();
   if (browseDirectory.isEmpty() == true)
   {
      browseDirectory = mBrowseDirectory;
   }

   // Get the filename from the user
   QString filename;
   if (mExistingFile == true)
   {
      filename = QFileDialog::getOpenFileName(this, mBrowseCaption, browseDirectory, mBrowseFilters);
   }
   else
   {
      filename = QFileDialog::getSaveFileName(this, mBrowseCaption, browseDirectory, mBrowseFilters);
   }

   if (filename.isEmpty() == false)
   {
      // Set the edit box text
      setFilename(filename);
   }
}
