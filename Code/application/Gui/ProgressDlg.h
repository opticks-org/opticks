/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROGRESSDLG_H
#define PROGRESSDLG_H

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>

#include "ProgressAdapter.h"

class Executable;
class QMovie;

class ProgressDlg : public QDialog
{
   Q_OBJECT

public:
   ProgressDlg(const QString& strCaption, QWidget* parent = NULL);
   ~ProgressDlg();

   void progressDeleted(Subject &subject, const std::string &signal, const boost::any &data);
   void progressUpdated(Subject &subject, const std::string &signal, const boost::any &data);

   void enableAbort(bool bEnable);
   bool hasAbort();
   bool isAborted();

public slots:
   virtual void reject();

signals:
   void aborted();

protected:
   void closeEvent(QCloseEvent* e);

protected slots:
   void setProcessComplete(bool bComplete);

private:
   QLabel* mpProgressLabel;
   QProgressBar* mpProgressBar;
   QPushButton* mpCancel;
   QWidget* mpWarningWidget;
   QTextEdit* mpWarningEdit;
   QMovie* mpProgressMovie;

   bool mbAutoClose;
   bool mbComplete;
   ProgressAdapter* mpProgressObject;
   Executable* mpPlugIn;

   bool mbHasAbort;
   bool mbAborted;
};

#endif
