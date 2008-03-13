/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef INTERACTIVEAPPLICATION_H__
#define INTERACTIVEAPPLICATION_H__

#include "Application.h"

class InteractiveApplication : public Application
{
public:
   InteractiveApplication(QCoreApplication &app) : Application (app) {};
   virtual ~InteractiveApplication() {};

   int run(int argc, char** argv);
};

#endif
