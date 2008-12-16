/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include <QtCore/QStringList>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QTextEdit>

class Progress;

class SearchDlg : public QDialog
{
   Q_OBJECT

public:
   SearchDlg(Progress* pProgress, QWidget* parent = 0);
   ~SearchDlg();

   QStringList getFilenames() const;
   QStringList getDirectories();
   bool usesSubdirectories() const;
   QStringList getTypes() const;

public slots:
   void setBrowseDirectory(const QString& strDirectory);
   void setSearchDirectories(QStringList directoryList);
   void setSubdirectories(bool bSubdirectories);
   void setTypes(QStringList typeList);
   void abortSearch();

protected:
   bool createDirectoryList(QStringList& directoryList);

protected slots:
   void accept();
   void browseDirectories();

private:
   Progress* mpProgress;
   bool mbAbort;

   QTextEdit* mpDirectoryEdit;
   QCheckBox* mpSubDirCheck;
   QListWidget* mpTypeList;

   QString mBrowseDirectory;
   QStringList mFileList;
};

#endif
