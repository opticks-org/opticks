/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCHEDITORDLG_H
#define BATCHEDITORDLG_H

#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QStackedWidget>

#include <vector>

class BatchFileset;
class BatchWizard;
class FilesetWidget;
class WizardWidget;

class BatchEditorDlg : public QMainWindow
{
   Q_OBJECT

public:
   BatchEditorDlg(QWidget* parent = 0);
   ~BatchEditorDlg();

   bool setBatchWizard(const QString& strXmlFilename);

protected:
   void closeEvent(QCloseEvent* e);

   bool promptForSave();
   BatchWizard* readWizardFile(const QString& strWizardFilename);
   void updateViewWidgets();

protected slots:
   void newWizard();
   void open();
   bool save();
   bool saveAs();
   bool executeBatch();
   bool executeInteractive();
   void add();
   void remove();
   void setView(QAction* pAction);
   void setWidgetValues(int iIndex);
   void setModified();

private:
   BatchEditorDlg(const BatchEditorDlg& rhs);
   BatchEditorDlg& operator=(const BatchEditorDlg& rhs);
   QString mXmlFilename;
   std::vector<BatchFileset*> mFilesets;
   std::vector<BatchWizard*> mWizards;

   QAction* mpFilesetAction;
   QAction* mpWizardAction;

   QLabel* mpViewLabel;
   QComboBox* mpViewCombo;

   QStackedWidget* mpStack;
   FilesetWidget* mpFilesetWidget;
   WizardWidget* mpWizardWidget;

   bool mbModified;

private:
   QStringList getFilesetNames() const;
   bool execute(bool bBatch);
   void destroyFilesets();
   void destroyWizards();
};

#endif   // BATCHEDITORDLG_H
