/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESDLG_H
#define PROPERTIESDLG_H

#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>

#include <vector>

class Properties;
class SessionItem;

class PropertiesDlg : public QDialog
{
   Q_OBJECT

public:
   PropertiesDlg(SessionItem* pSessionItem, const std::vector<std::string>& displayedPages, QWidget* pParent = NULL);
   ~PropertiesDlg();

protected:
   bool applyChanges();

protected slots:
   void processButtonClick(QAbstractButton* pButton);

private:
   SessionItem* mpSessionItem;
   std::vector<Properties*> mPlugIns;
   QDialogButtonBox* mpButtonBox;
};

#endif
