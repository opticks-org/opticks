/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INSTALLWIZARD_H
#define INSTALLWIZARD_H

#include <QtCore/QList>
#include <QtGui/QWizard>

class Aeb;
class Progress;
class QCloseEvent;

class InstallWizard : public QWizard
{
   Q_OBJECT

public:
   InstallWizard(QList<Aeb*>& packageDescriptors, Progress* pProgress = NULL, QWidget* pParent = NULL);
   virtual ~InstallWizard();

public slots:
   void reject();

protected:
   virtual void closeEvent(QCloseEvent* pEvent);

private slots:
   void install();

private:
   InstallWizard(const InstallWizard& rhs);
   InstallWizard& operator=(const InstallWizard& rhs);
   QList<Aeb*> mPackageDescriptors;
   Progress* mpProgress;
};

#endif
