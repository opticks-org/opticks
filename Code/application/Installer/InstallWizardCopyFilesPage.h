/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INSTALLWIZARDCOPYFILESPAGE_H
#define INSTALLWIZARDCOPYFILESPAGE_H

#include <QtCore/QList>
#include <QtGui/QWizardPage>

class Aeb;
class Progress;

class InstallWizardCopyFilesPage : public QWizardPage
{
   Q_OBJECT

public:
   InstallWizardCopyFilesPage(QList<Aeb*>& packageDescriptors, Progress* pProgress, QWidget* pParent = NULL);
   virtual ~InstallWizardCopyFilesPage();

   virtual bool validatePage();

private:
   QList<Aeb*> mPackageDescriptors;
   Progress* mpProgress;
};

#endif
