/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODELESS_H
#define MODELESS_H

#include <QtGui/QDialog>

#include "ViewerShell.h"

class ModelessPlugIn : public ViewerShell
{
public:
   ModelessPlugIn();
   ~ModelessPlugIn();

   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   QWidget* getWidget() const;

private:
   QDialog* mpDialog;
   int mRuns;
   bool mSessionClosed;

   void aboutToClose(Subject& subject, const std::string& signal, const boost::any &args);
   void updateConfigSettings(Subject& subject, const std::string& signal, const boost::any &args);
   void sessionClosed(Subject& subject, const std::string& signal, const boost::any& value);
};


class ModelessDlg : public QDialog
{
   Q_OBJECT

public:
   ModelessDlg(PlugIn* pPlugIn, QWidget* pParent = 0);
   ~ModelessDlg();

protected:
   void closeEvent(QCloseEvent* pEvent);

private:
   PlugIn* mpPlugIn;
};

#endif
