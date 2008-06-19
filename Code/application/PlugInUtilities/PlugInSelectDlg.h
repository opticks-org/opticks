/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PLUGINSELECTDLG_H
#define PLUGINSELECTDLG_H

#include <QtCore/QString>
#include <QtGui/QDialog>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include "TypesFile.h"

#include <vector>

class PlugInSelectDlg : public QDialog
{
   Q_OBJECT

public:
   PlugInSelectDlg(QWidget* parent = 0);
   ~PlugInSelectDlg();

   void setDisplayedPlugInType(const std::string& type, const std::string& strSubtype = "");
   void setDisplayedPlugInTypes(const std::vector<std::string>& types, const std::string& strSubtype = "");
   void setExcludedPlugInTypes(const std::vector<std::string>& types, const std::string& strSubtype = "");
   void setShowPlugInsForWizardOnly(bool show);

   std::string getSelectedPlugInName() const;
   std::string getSelectedPlugInType() const;

protected slots:
   void updatePlugInList();
   void acceptItem(QTreeWidgetItem* pItem);
   void accept();

private:
   std::vector<std::string> mTypes;
   std::vector<std::string> mExcludedTypes;
   std::string mSubtype;
   std::string mExcludedSubtype;
   bool mShowWizardOnly;

   QTreeWidget* mpPlugInList;
};

#endif
