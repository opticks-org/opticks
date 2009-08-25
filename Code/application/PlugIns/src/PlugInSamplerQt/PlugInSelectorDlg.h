/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINSELECTORDLG_H
#define PLUGINSELECTORDLG_H

#include <QtGui/QDialog>

#include <string>
#include <vector>

class QCheckBox;
class QComboBox;

class PlugInSelectorDlg : public QDialog
{
   Q_OBJECT

public:
   PlugInSelectorDlg(QWidget* pParent, const std::vector<std::string>& plugins);
   
   std::vector<std::string> getSelectedPlugins() const;

private slots:
   void enableRunIndivTest(bool bRunAll);

private:
   bool runAllTests() const;

   QComboBox* mpCombo;
   QCheckBox* mpRunAll;
};

#endif
