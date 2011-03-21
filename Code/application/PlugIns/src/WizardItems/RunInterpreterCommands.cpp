/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "Filename.h"
#include "Interpreter.h"
#include "InterpreterManager.h"
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "ProgressTracker.h"
#include "RunInterpreterCommands.h"
#include "Slot.h"
#include "StringUtilities.h"

#include <boost/any.hpp>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include <string>
#include <vector>

REGISTER_PLUGIN_BASIC(OpticksWizardItems, RunInterpreterCommands);

RunInterpreterCommands::RunInterpreterCommands() : mpStream(NULL), mVerbose(false), mpProgress(NULL)
{
   setName("Run Interpreter Commands");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Run one or more commands through an interpreter.");
   setDescriptorId("{6E45BC1B-114C-47e0-A940-63E3CB0E51BF}");
   allowMultipleInstances(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

RunInterpreterCommands::~RunInterpreterCommands()
{}

bool RunInterpreterCommands::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(WizardItems::getInputSpecification(pArgList) && pArgList != NULL);
   VERIFY(pArgList->addArg<std::string>("Interpreter Name",
      "The name of the interpreter to use. An existing instance will be used whenever possible. May not be empty."));
   VERIFY(pArgList->addArg<std::string>("Command",
      "A single command for the interpreter to run. May not be used with any other command argument."));
   VERIFY(pArgList->addArg<std::vector<std::string> >("Command List",
      "A list of commands for the interpreter to run. May not be used with any other command argument."));
   VERIFY(pArgList->addArg<Filename>("Command File",
      "A file with commands for the interpreter to run. May not be used with any other command argument."));
   VERIFY(pArgList->addArg<Filename>("Log File",
      "An optional file for interpreter output. This file will be overwritten if it exists."));
   VERIFY(pArgList->addArg<std::string>("Command File Location",
      "A file with commands for the interpreter to run.  You can use $V(APP_HOME) inside this "
      "string and it will auto-expand to the application install location.  May not be used with any other "
      "command argument."));
   VERIFY(pArgList->addArg<bool>("Verbose", false,
      "If true, normal output from the interpreter will be reported to the progress object as a warning. "
      "If false, which is the default, normal output from the interpreter will not appear as progress."));
   return true;
}

bool RunInterpreterCommands::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool RunInterpreterCommands::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Run Interpreter Commands.", "app", "FBA8DDA3-9EA4-40da-BC78-CCD55E867297");

   mVerbose = false;
   pInArgList->getPlugInArgValue<bool>("Verbose", mVerbose);

   // Get the command(s) to send to the interpreter.
   // Commands should be validated before Interpreter Name to avoid potentially creating a plug-in without need.
   std::string command;
   const bool hasCommand = pInArgList->getPlugInArgValue("Command", command);

   std::vector<std::string> commandList;
   const bool hasCommandList = pInArgList->getPlugInArgValue("Command List", commandList);

   std::string commandFileLocation;
   const bool hasCommandFileLocation = pInArgList->getPlugInArgValue("Command File Location" , commandFileLocation);

   Filename* const pCommandFile = pInArgList->getPlugInArgValue<Filename>("Command File");
   const bool hasCommandFile = pCommandFile != NULL;

   // Check that exactly one command type was specified by the user.
   if ((hasCommand && hasCommandList) ||
       (hasCommand && hasCommandFile) ||
       (hasCommandList && hasCommandFile) ||
       (hasCommand && hasCommandFileLocation) ||
       (hasCommandList && hasCommandFileLocation) ||
       (hasCommandFile && hasCommandFileLocation) ||
       (hasCommand == false && hasCommandList == false && hasCommandFile == false && hasCommandFileLocation == false))
   {
      progress.report("Exactly one type of command must be specified.", 0, ERRORS, true);
      return false;
   }

   // Marshal the command input into the commandList std::vector for processing.
   if (hasCommand)
   {
      commandList.push_back(command);
   }

   std::string commandFilePath;
   if (hasCommandFile || hasCommandFileLocation)
   {
      // Read the file, putting each line (including empty lines) into its own command.
      // This is done to close the source file as quickly as possible so that the file can be edited while commands
      // are being processed even though it requires more memory and errors early in the file will not appear as
      // quickly because the whole file is read before any commands are sent to the interpreter.
      if (hasCommandFile)
      {
         commandFilePath = pCommandFile->getFullPathAndName();
      }
      else
      {
         commandFilePath = StringUtilities::expandVariables(commandFileLocation);
      }
      QFile commandFile(QString::fromStdString(commandFilePath));
      if (commandFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
      {
         progress.report("Unable to read from the command file at: \"" +
            commandFilePath + "\".", 0, ERRORS, true);
         return false;
      }

      while (commandFile.atEnd() == false)
      {
         // Read a whole line, stripping off any newline characters.
         QString line = commandFile.readLine();
         if (line.endsWith("\n") == true)
         {
            line.chop(1);
         }

         // Add this command into the std::vector of commands.
         commandList.push_back(line.toStdString());
      }
   }

   // Ensure that at least one command is present.
   // The value of the return here is arbitrary, but the check must be done to avoid a possible division by 0 later.
   if (commandList.empty() == true)
   {
      progress.report("At least one command must be specified.", 0, ERRORS, true);
      return false;
   }

   // Execute each command.
   std::string totalCommand;
   for (std::vector<std::string>::iterator iter = commandList.begin(); iter != commandList.end(); ++iter)
   {
      totalCommand += *iter;
      if (iter != commandList.end())
      {
         totalCommand += "\n";
      }
   }

   // Check for log file presence.
   mpStream = NULL;
   QFile logFile;
   QTextStream logStream;
   Filename* const pLogFile = pInArgList->getPlugInArgValue<Filename>("Log File");
   if (pLogFile != NULL)
   {
      // Open the log file.
      logFile.setFileName(QString::fromStdString(pLogFile->getFullPathAndName()));
      if (logFile.open(QIODevice::WriteOnly | QIODevice::Text) == true)
      {
         // Use the insertion operator for convenience.
         logStream.setDevice(&logFile);

         // Write the current date and time along with the total number of commands to process.
         logStream << QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate) << " (local time).<br>\n<br>\n";
         mpStream = &logStream;
      }
      else
      {
         // If the file cannot be opened for writing, log a warning and continue.
         progress.report("Unable to write to the log file at: \"" +
            pLogFile->getFullPathAndName() + "\". Logging is disabled.", 0, WARNING, true);
      }
   }

   // Get the name of the interpreter and either use an existing instance or create one.
   // If ScriptingWindow has ever been opened, then it will have loaded all interpreter plug-ins.
   // In this case, latch onto an existing instance of the plug-in. Otherwise, create one for this wizard item to use.
   // Disallow an empty interpreter name as some non-interpreter plug-ins have an empty name.
   std::string interpreterName;
   std::vector<PlugIn*> plugIns;
   InterpreterManager* pInterMgr = NULL;
   if (!pInArgList->getPlugInArgValue("Interpreter Name", interpreterName) && !commandFilePath.empty())
   {
      //no interpreter name provided, deduce from commandFilePath
      std::vector<PlugInDescriptor*> descriptors = Service<PlugInManagerServices>()->getPlugInDescriptors(
         PlugInManagerServices::InterpreterManagerType());
      std::string foundInterpreter;
      for (std::vector<PlugInDescriptor*>::iterator iter = descriptors.begin();
         iter != descriptors.end();
         ++iter)
      {
         if (checkExtension(*iter, commandFilePath))
         {
            interpreterName = (*iter)->getName();
            break;
         }
      }
   }
   if (interpreterName.empty() == false)
   {
      plugIns = Service<PlugInManagerServices>()->getPlugInInstances(interpreterName);
      if (!plugIns.empty())
      {
         pInterMgr = dynamic_cast<InterpreterManager*>(plugIns.front());
      }
      else
      {
         PlugInResource res(interpreterName);
         pInterMgr = dynamic_cast<InterpreterManager*>(res.get());
         res.release();
      }
   }

   if (pInterMgr == NULL)
   {
      std::string errorMessage;
      std::vector<PlugInDescriptor*> plugInDescriptors =
         Service<PlugInManagerServices>()->getPlugInDescriptors(PlugInManagerServices::InterpreterManagerType());
      if (plugInDescriptors.empty() == true)
      {
         errorMessage = "No interpreters exist. Please check your installation and try again.";
      }
      else
      {
         if (interpreterName.empty() == true)
         {
            errorMessage = "No interpreter specified.";
         }
         else
         {
            errorMessage = "Interpreter \"" + interpreterName + "\" not found.";
         }

         errorMessage += " Available interpreters: ";
         for (std::vector<PlugInDescriptor*>::const_iterator iter = plugInDescriptors.begin();
            iter != plugInDescriptors.end();
            ++iter)
         {
            VERIFY(*iter != NULL);
            errorMessage += "\"" + (*iter)->getName() + "\", ";
         }

         // Replace the final comma with a period.
         *(errorMessage.rbegin() + 1) = '.';
      }

      progress.report(errorMessage, 0, ERRORS, true);
      return false;
   }

   if (!pInterMgr->isStarted())
   {
      pInterMgr->start();
      std::string startupMsg = pInterMgr->getStartupMessage();
      if (logStream.device() != NULL)
      {
         logStream << QString::fromStdString(startupMsg) << "<br>\n";
      }
      progress.report(startupMsg, 0, WARNING, true);
   }
   Interpreter* pInterpreter = pInterMgr->getInterpreter();

   mpProgress = progress.getCurrentProgress();
   if (pInterpreter == NULL)
   {
      std::string errorMessage = "Interpreter could not be started.";
      if (logStream.device() != NULL)
      {
         logStream << QString::fromStdString(errorMessage) << "<br>\n";
      }

      progress.report(errorMessage, 0, ERRORS, true);
      return false;
   }
   if (logStream.device() != NULL)
   {
      std::string prompt = pInterpreter->getPrompt();
      logStream << QString::fromStdString(prompt) << QString::fromStdString(totalCommand) << "<br>\n";
   }

   bool retValue = pInterpreter->executeScopedCommand(totalCommand,
      Slot(this, &RunInterpreterCommands::receiveStandardOutput),
      Slot(this, &RunInterpreterCommands::receiveErrorOutput),
      mpProgress);

   mpStream = NULL;
   mpProgress = NULL;

   if (retValue == false)
   {
      std::string message = "Error running command";

      progress.report(message, 0, ERRORS, true);
      progress.upALevel();
      return false;
   }

   progress.report("Running interpreter commands.", 100, NORMAL);
   progress.upALevel();
   return true;
}

void RunInterpreterCommands::receiveStandardOutput(Subject& subject, const std::string& signal, const boost::any& data)
{
   receiveOutput(data, false);
}

void RunInterpreterCommands::receiveErrorOutput(Subject& subject, const std::string& signal, const boost::any& data)
{
   receiveOutput(data, true);
}

void RunInterpreterCommands::receiveOutput(const boost::any& data, bool isErrorText)
{
   std::string text = boost::any_cast<std::string>(data);

   if (text.empty())
   {
      return;
   }
   if (mpStream != NULL)
   {
      const QString color(isErrorText ? "red" : "green");
      *mpStream << "<font color=\"" << color << "\">" << QString::fromStdString(text) << "</font><br>\n";

      // Flush after each command.
      mpStream->flush();
   }

   if (mpProgress != NULL && (isErrorText || mVerbose))
   {
      std::string ignoreText;
      int currentProgress;
      ReportingLevel ignoreLevel;
      mpProgress->getProgress(ignoreText, currentProgress, ignoreLevel);
      mpProgress->updateProgress(text, currentProgress, WARNING);
   }
}

bool RunInterpreterCommands::checkExtension(PlugInDescriptor* pDescriptor, const std::string& filename)
{
   bool bMatch = false;
   if (pDescriptor == NULL)
   {
      return false;
   }
   QString filePath = QString::fromStdString(filename);
   QString strInterpreterExtensions = QString::fromStdString(pDescriptor->getFileExtensions());
   if (!strInterpreterExtensions.isEmpty())
   {
      QStringList filterListCandidates = strInterpreterExtensions.split(";;", QString::SkipEmptyParts);
      QStringList filterList;
      for (int i = 0; i < filterListCandidates.count(); i++)
      {
         QString strExtensions = filterListCandidates[i];
         if (strExtensions.isEmpty() == false)
         {
            int iOpenPos = strExtensions.indexOf("(");
            int iClosePos = strExtensions.lastIndexOf(")");
            strExtensions = strExtensions.mid(iOpenPos + 1, iClosePos - iOpenPos - 1);

            QStringList globPatterns = strExtensions.split(QString(" "), QString::SkipEmptyParts);
            QString catchAll = QString::fromStdString("*");
            QString catchAll2 = QString::fromStdString("*.*");
            for (int globCount = 0; globCount < globPatterns.count(); ++globCount)
            {
               QString pattern = globPatterns[globCount];
               if ((pattern != catchAll) && (pattern != catchAll2))
               {
                  filterList << pattern;
               }
            }
         }
      }

      bMatch = QDir::match(filterList, filePath);
   }
   return bMatch;
}
