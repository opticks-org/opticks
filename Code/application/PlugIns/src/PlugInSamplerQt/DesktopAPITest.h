/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DESKTOPAPITEST_H
#define DESKTOPAPITEST_H

#include <QtCore/QObject>

#include "ViewerShell.h"

class DesktopAPITestGui;

class DesktopAPITest : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   DesktopAPITest();
   ~DesktopAPITest();

   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   QWidget* getWidget() const;

protected slots:
   void dialogClosed();

private:
   DesktopAPITest(const DesktopAPITest& rhs);
   DesktopAPITest& operator=(const DesktopAPITest& rhs);
   DesktopAPITestGui* mpGui;
};

#endif
