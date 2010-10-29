/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RUNINTERPRETERCOMMANDS_H
#define RUNINTERPRETERCOMMANDS_H

#include "WizardItems.h"

#include <boost/any.hpp>
#include <string>

class PlugInDescriptor;
class Progress;
class QTextStream;

class RunInterpreterCommands : public WizardItems
{
public:
   RunInterpreterCommands();
   virtual ~RunInterpreterCommands();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
private:
   void receiveStandardOutput(Subject& subject, const std::string& signal, const boost::any& data);
   void receiveErrorOutput(Subject& subject, const std::string& signal, const boost::any& data);
   void receiveOutput(const boost::any& data, bool isErrorText);
   bool checkExtension(PlugInDescriptor* pDescriptor, const std::string& filename);
   QTextStream* mpStream;
   bool mVerbose;
   Progress* mpProgress;
};

#endif
