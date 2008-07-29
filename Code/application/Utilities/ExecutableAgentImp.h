/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXECUTABLE_AGENT_IMP_H
#define EXECUTABLE_AGENT_IMP_H

#include <string>

#include "PlugInResource.h"

class Executable;
class PlugInArgList;
class Progress;
class Window;

class ExecutableAgentImp
{
public:
   ExecutableAgentImp();
   virtual void instantiate(Progress* pProgress, bool batch);
   virtual void instantiate(const std::string& plugInName, const std::string& menuCommand,
      Progress* pProgress, bool batch);
   virtual void instantiate(PlugIn* pPlugIn, const std::string& menuCommand,
      Progress* pProgress, bool batch);

   virtual ~ExecutableAgentImp();
   virtual void setPlugIn(const std::string& plugInName);
   virtual void setPlugIn(PlugIn* pPlugIn);
   virtual Progress* getProgress() const;
   virtual void createProgressDialog(bool bCreate);
   virtual bool isProgressDialogCreated() const;
   virtual bool execute();
   virtual bool abort();
   virtual bool runAllTests(Progress *pProgress, std::ostream &output);
   virtual PlugInArgList &getInArgList() const;
   virtual PlugInArgList &getPopulatedInArgList();
   virtual PlugInArgList &getOutArgList() const;
   virtual const PlugIn* getPlugIn() const;
   virtual PlugIn* getPlugIn();
   virtual PlugIn* releasePlugIn();
   virtual void setAutoArg(bool bAutoArg);
   virtual bool getAutoArg() const;
protected:
   void checkInstantiate() const;

   /**
    *  Creates the input and output arg lists for the plug-in.
    *
    *  This method creates both input and output arg lists.
    *  This method does nothing if the arg lists have already been created.
    *  NOTE: The input argument list in not populated with non-default values. If
    *  you need populated input argument list, call getInArgList().
    *
    *  When this function returns, mpInArgList and mpOutArgList will not be NULL.
    *
    *  @see getInArgList(), populateArgValues()
    */
   virtual void setupArgList();

   /**
    *  Populates values into the input arg list.
    *
    *  @param   pArgList
    *           A pointer to the input arg list.
    */
   virtual void populateArgValues(PlugInArgList *pArgList);

   bool getInstantiated() const;
   bool isBatch() const;
   void createProgressDialog();
   bool executePlugIn();

private:
   void clearArgLists(bool clearInputArgList = true, bool clearOutputArgList = true);

   bool mInstantiated;
   bool mBatch;
   bool mSupportsRequestedBatchSetting;
   PlugInArgList* mpInArgList;
   PlugInArgList* mpOutArgList;
   std::string mMenuCommand;
   Progress* mpProgress;
   bool mProgressDialog;
   PlugInResource mPlugIn;
   bool mAutoInArg;
};

#define EXECUTABLEAGENTADAPTER_METHODS(impClass) \
   void instantiate(Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(pProgress, batch); \
   } \
   void instantiate(const std::string& plugInName, const std::string& menuCommand, \
      Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(plugInName, menuCommand, pProgress, batch); \
   } \
   void instantiate(PlugIn* pPlugIn, const std::string& menuCommand, \
      Progress* pProgress, bool batch) \
   { \
      impClass::instantiate(pPlugIn, menuCommand, pProgress, batch); \
   } \
   void setPlugIn(const std::string& plugInName) \
   { \
      impClass::setPlugIn(plugInName); \
   } \
   void setPlugIn(PlugIn* pPlugIn) \
   { \
      impClass::setPlugIn(pPlugIn); \
   } \
   Progress* getProgress() const \
   { \
      return impClass::getProgress(); \
   } \
   void createProgressDialog(bool bCreate) \
   { \
      impClass::createProgressDialog(bCreate); \
   } \
   bool isProgressDialogCreated() const \
   { \
      return impClass::isProgressDialogCreated(); \
   } \
   bool execute() \
   { \
      return impClass::execute(); \
   } \
   bool abort() \
   { \
      return impClass::abort(); \
   } \
   bool runAllTests(Progress *pProgress, std::ostream &output) \
   { \
      return impClass::runAllTests(pProgress, output); \
   } \
   PlugInArgList &getInArgList() const \
   { \
      return impClass::getInArgList(); \
   } \
   PlugInArgList &getOutArgList() const \
   { \
      return impClass::getOutArgList(); \
   } \
   PlugIn* getPlugIn() \
   { \
      return impClass::getPlugIn(); \
   } \
   const PlugIn* getPlugIn() const \
   { \
      return impClass::getPlugIn(); \
   } \
   PlugIn* releasePlugIn() \
   { \
      return impClass::releasePlugIn(); \
   } \
   void setAutoArg(bool bAutoArg) \
   { \
      impClass::setAutoArg(bAutoArg); \
   } \
   bool getAutoArg() const \
   { \
      return impClass::getAutoArg(); \
   }

#endif
