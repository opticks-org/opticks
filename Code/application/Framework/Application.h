/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>

class Progress;
class QCoreApplication;

class Application
{
public:
   virtual ~Application();

   virtual int run(int argc, char** argv);      // Must be re-implemented in subclasses!

   QCoreApplication& getQApp()
   {
      return mApplication;
   }

   virtual void reportWarning(const std::string& warningMessage) const = 0;
   virtual void reportError(const std::string& errorMessage) const = 0;

protected:
   Application(QCoreApplication& app);

   bool isXmlInitialized() const;
   bool generateXml();
   bool executeStartupBatchWizards();

   Progress* mpProgress;

private:
   QCoreApplication& mApplication;
   bool mXmlInitialized;
};

#endif
