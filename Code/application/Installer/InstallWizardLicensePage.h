/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INSTALLWIZARDLICENSEPAGE_H
#define INSTALLWIZARDLICENSEPAGE_H

#include <QtGui/QWizardPage>

class Aeb;

class InstallWizardLicensePage : public QWizardPage
{
   Q_OBJECT

public:
   InstallWizardLicensePage(const Aeb* pPackageDescriptor,
      unsigned int licenseNum, const QString& license, bool isHtml, QWidget* pParent = NULL);
   virtual ~InstallWizardLicensePage();

private:
   InstallWizardLicensePage(const InstallWizardLicensePage& rhs);
   InstallWizardLicensePage& operator=(const InstallWizardLicensePage& rhs);
};

#endif
