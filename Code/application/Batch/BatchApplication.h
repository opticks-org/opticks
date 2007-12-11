/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCH_H__
#define BATCH_H__

#include "Application.h"
#include "SubjectAdapter.h"

#include <string>

class BatchApplication : public Application, public SubjectAdapter
{
public:
   BatchApplication(QCoreApplication& app);
   virtual ~BatchApplication();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   virtual int run(int argc, char** argv);
   virtual int test(int argc, char** argv);
   virtual int version(int argc, char** argv);

protected:
   void log(const std::string& message,
            std::string component,
            std::string key);
};

#endif
