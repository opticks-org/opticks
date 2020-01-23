/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WINDOWSAEBINSTALLAPPLICATION_H
#define WINDOWSAEBINSTALLAPPLICATION_H

#include "InteractiveApplication.h"

#include <string>

class WindowsAEBInstallApplication : public InteractiveApplication
{
public:
   WindowsAEBInstallApplication(QCoreApplication& app);
   virtual ~WindowsAEBInstallApplication();

   virtual int run(int argc, char** argv);

private:
   WindowsAEBInstallApplication& operator=(const WindowsAEBInstallApplication& rhs);
};

#endif
