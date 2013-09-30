/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSDLG_H
#define OPTIONSDLG_H

#include <QtGui/QDialog>

#include <map>
#include <vector>

class Option;
class QLabel;
class QSplitter;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;

class OptionsDlg : public QDialog
{
   Q_OBJECT

public:
   OptionsDlg(QWidget* pParent = NULL);
   virtual ~OptionsDlg();

   void activatePage(const QString& pageName);

public slots:
   virtual void accept();

protected slots:
   void updateOptionWidget();

private:
   OptionsDlg(const OptionsDlg& rhs);
   OptionsDlg& operator=(const OptionsDlg& rhs);

   void populateDialogWithOptions();
   void saveState();
   void restoreState();
   bool setCurrentOptionByPath(std::string path);
   std::string getCurrentOptionPath();

   QSplitter* mpSplitter;
   QTreeWidget* mpOptionSelection;
   std::vector<Option*> mOptionPlugIns;
   std::map<QTreeWidgetItem*, QWidget*> mOptionWidgets;
   QLabel* mpNoOptionsWidget;
   QStackedWidget* mpOptionStack;
};

#endif
