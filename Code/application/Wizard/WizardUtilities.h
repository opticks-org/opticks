/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDUTILITIES_H
#define WIZARDUTILITIES_H

#include <string>
#include <vector>

class BatchWizard;
class Progress;
class WizardObject;

namespace WizardUtilities
{
   /**
    * Derives the filename of the batch wizard from a given wizard's filename.
    *
    * @param wizardFilename
    *        The full path to a .wiz file
    *
    * @return Returns the full path to the corresponding .batchwiz.  The path returned may or may
    *         not exist.
    */
   std::string deriveBatchWizardFilename(const std::string& wizardFilename);

   /**
    * Writes the given BatchWizard objects to the given .batchwiz file.
    *
    * @param batchWizards
    *        The list of BatchWizard objects to be written to the file.
    * @param batchFilename
    *        The full path that the BatchWizard objects should be written to, ie. the .batchwiz file.
    *
    * @return Returns true if the .batchwiz file was successfully written, false otherwise.
    */
   bool writeBatchWizard(const std::vector<BatchWizard*>& batchWizards, const std::string& batchFilename);

   /**
    * Creates a BatchWizard from the provided wizard (ie. .wiz) file.
    *
    * @param wizardFilename
    *        The full path to the .wiz file for which a BatchWizard object should be created.
    *
    * @return Returns a created BatchWizard object that was derived from the given wizard file
    *         or NULL if unsuccessful.
    */
   BatchWizard* createBatchWizardFromWizard(const std::string& wizardFilename);

   /**
    * Creates a BatchWizard based upon a WizardObject.
    * 
    * @param pWizard
    *        The wizard object to create a batch wizard for.
    *
    * @return The newly created batch wizard based on the WizardObject.
    */
   BatchWizard* createBatchWizardFromWizard(WizardObject* pWizard, const std::string& wizardFilename);

   /**
    * Reads the given .wiz file and returns a WizardObject
    * if the file could be read successfully.
    * 
    * @param wizardFilename
    *        The full path to the .wiz file to parse.
    *
    * @return Returns the parsed wizard file or NULL if there
    *         was a parsing error.
    */
   WizardObject* readWizard(const std::string& wizardFilename);

   /**
    * Executes the Wizard Executor plug-in with the given wizard element.
    *
    * This method launches the given wizard.  A progress object and dialog are created for
    * the wizard executor plug-in.
    *
    * @param   pWizard
    *          The wizard element to execute.
    */
   void runWizard(WizardObject* pWizard);

   /**
    * Executes the given batch wizard files (ie. .batchwiz).
    *
    * @param batchWizardFiles
    *        The list of .batchwiz files that should be executed.
    * @param pProgress
    *        The progress object that should be used to report progress
    *        on the batch wizard execution.  NULL can be provided in which
    *        case no progress will be reported.
    * 
    * @return Returns true if execution of the batch wizard files could be started, false otherwise.
    */
   bool runBatchFiles(const std::vector<std::string>& batchWizardFiles, Progress* pProgress);
};

#endif
