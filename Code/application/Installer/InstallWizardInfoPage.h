/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INSTALLWIZARDINFOPAGE_H
#define INSTALLWIZARDINFOPAGE_H

#include <QtCore/QList>
#include <QtGui/QWizardPage>

class Aeb;
class QListWidget;

class InstallWizardInfoPage : public QWizardPage
{
   Q_OBJECT

public:
   InstallWizardInfoPage(const QList<Aeb*>& packageDescriptors, QWidget* pParent = NULL);
   virtual ~InstallWizardInfoPage();
   bool eventFilter(QObject* pObject, QEvent* pEvent);


private:
   InstallWizardInfoPage(const InstallWizardInfoPage& rhs);
   InstallWizardInfoPage& operator=(const InstallWizardInfoPage& rhs);
   QListWidget* mpList;
};

#endif
