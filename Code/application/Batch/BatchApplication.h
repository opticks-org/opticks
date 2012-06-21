/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCHAPPLICATION_H
#define BATCHAPPLICATION_H

#include "Application.h"
#include "SubjectAdapter.h"

#include <string>

class BatchApplication : public Application, public SubjectAdapter
{
public:
   BatchApplication(QCoreApplication& app);
   ~BatchApplication();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   int run(int argc, char** argv);
   int test(int argc, char** argv);
   int version(int argc, char** argv);

   void reportWarning(const std::string& warningMessage) const;
   void reportError(const std::string& errorMessage) const;

private:
   BatchApplication& operator=(const BatchApplication& rhs);
};

#endif
