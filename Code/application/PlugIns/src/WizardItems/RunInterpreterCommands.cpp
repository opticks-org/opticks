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
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "ProgressTracker.h"
#include "RunInterpreterCommands.h"
#include "StringUtilities.h"

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include <string>
#include <vector>

REGISTER_PLUGIN_BASIC(OpticksWizardItems, RunInterpreterCommands);

RunInterpreterCommands::RunInterpreterCommands()
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
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(ProgressArg()),
      "Run Interpreter Commands.", "app", "FBA8DDA3-9EA4-40da-BC78-CCD55E867297");

   bool verbose = false;
   pInArgList->getPlugInArgValue<bool>("Verbose", verbose);

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

   if (hasCommandFile || hasCommandFileLocation)
   {
      // Read the file, putting each line (including empty lines) into its own command.
      // This is done to close the source file as quickly as possible so that the file can be edited while commands
      // are being processed even though it requires more memory and errors early in the file will not appear as
      // quickly because the whole file is read before any commands are sent to the interpreter.
      std::string commandFilePath;
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

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Should this create instances (OPTICKS-815)? (dadkins)")
   // Get the name of the interpreter and either use an existing instance or create one.
   // If ScriptingWindow has ever been opened, then it will have loaded all interpreter plug-ins.
   // In this case, latch onto an existing instance of the plug-in. Otherwise, create one for this wizard item to use.
   // Disallow an empty interpreter name as some non-interpreter plug-ins have an empty name.
   std::string interpreterName;
   std::vector<PlugIn*> plugIns;
   if (pInArgList->getPlugInArgValue("Interpreter Name", interpreterName) == true && interpreterName.empty() == false)
   {
      plugIns = Service<PlugInManagerServices>()->getPlugInInstances(interpreterName);
   }

   ExecutableResource pInterpreterResource(plugIns.empty() == true ? NULL : plugIns.front(),
      std::string(), progress.getCurrentProgress(), isBatch());
   if (pInterpreterResource->getPlugIn() != NULL)
   {
      // If the plug-in was found, then it is owned by another object (most likely ScriptingWidget).
      // If this instance is not released here the plug-in will be unloaded twice, resulting in a crash.
      pInterpreterResource->releasePlugIn();

      // Interpreters that support multiple instances may not work correctly in all cases.
      if (pInterpreterResource->getPlugIn()->areMultipleInstancesAllowed() == true)
      {
         progress.report("The plug-in specified supports multiple instances. "
            "An existing instance was chosen.", 0, WARNING, true);
      }
   }
   else
   {
      // If the plug-in was not found, then create it for this wizard item, destroying it upon completion.
      // Disallow an empty interpreter name as some non-interpreter plug-ins have an empty name.
      if (interpreterName.empty() == false)
      {
         pInterpreterResource->setPlugIn(interpreterName);
      }
   }

   // Check that the plug-in either exists or was created successfully.
   // If not, report an error and exit immediately.
   Interpreter* const pInterpreter = dynamic_cast<Interpreter*>(pInterpreterResource->getPlugIn());
   if (pInterpreter == NULL)
   {
      std::string errorMessage;
      std::vector<PlugInDescriptor*> plugInDescriptors =
         Service<PlugInManagerServices>()->getPlugInDescriptors(PlugInManagerServices::InterpreterType());
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

   // Check for log file presence.
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
         logStream << QDateTime::currentDateTime().toString(Qt::SystemLocaleLongDate) << " (local time).<br>\n" <<
            "Number of commands: " << commandList.size() << "<br>\n<br>\n";
      }
      else
      {
         // If the file cannot be opened for writing, log a warning and continue.
         progress.report("Unable to write to the log file at: \"" +
            pLogFile->getFullPathAndName() + "\". Logging is disabled.", 0, WARNING, true);
      }
   }

   // Execute each command.
   double percent = 0;
   const double step = 99.0 / commandList.size();
   for (std::vector<std::string>::iterator iter = commandList.begin(); iter != commandList.end(); ++iter)
   {
      if (isAborted() == true)
      {
         progress.report("Aborted", 0, ABORT, true);
         return false;
      }

      progress.report("Running interpreter commands.", static_cast<int>(percent), NORMAL);
      std::string prompt = pInterpreter->getPrompt();
      pInterpreterResource->getInArgList().setPlugInArgValue<std::string>(Interpreter::CommandArg(), &(*iter));
      const bool success = pInterpreterResource->execute();
      percent += step;

      const PlugInArgList& outArgList = pInterpreterResource->getOutArgList();
      const std::string* const pOutputText = outArgList.getPlugInArgValue<std::string>(Interpreter::OutputTextArg());
      const std::string* const pReturnType = outArgList.getPlugInArgValue<std::string>(Interpreter::ReturnTypeArg());
      if (logStream.device() != NULL)
      {
         // Write the results of this command to the log file.
         logStream << QString::fromStdString(prompt) << QString::fromStdString(*iter) << "<br>\n";
         if (pOutputText != NULL && pOutputText->empty() == false)
         {
            const QString color((pReturnType != NULL && *pReturnType == "Error") ? "red" : "green");
            logStream << "<font color=\"" << color << "\">" << QString::fromStdString(*pOutputText) << "</font><br>\n";
         }

         // Flush after each command.
         logStream.flush();
      }

      if (success == false || (pReturnType != NULL && *pReturnType == "Error"))
      {
         std::string message = "Error running command \"" + *iter + "\". ";
         if (pOutputText != NULL)
         {
            message += *pOutputText;
         }

         progress.report(message, 0, ERRORS, true);
         return false;
      }

      if (verbose && pReturnType != NULL && *pReturnType != "Error" && pOutputText != NULL && !pOutputText->empty())
      {
         std::string ignoreText;
         int currentProgress;
         ReportingLevel ignoreLevel;
         Progress* pProgress = progress.getCurrentProgress();
         if (pProgress != NULL)
         {
            pProgress->getProgress(ignoreText, currentProgress, ignoreLevel);
            pProgress->updateProgress(*pOutputText, currentProgress, WARNING);
         }
      }
   }

   progress.report("Running interpreter commands.", 100, NORMAL);
   progress.upALevel();
   return true;
}
