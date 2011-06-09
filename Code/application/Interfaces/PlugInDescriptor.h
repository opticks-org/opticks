/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINDESCRIPTOR_H
#define PLUGINDESCRIPTOR_H

#include "SessionItem.h"

#include <map>
#include <string>
#include <vector>

class PlugInArgList;

/**
 * Caches plug-in metadata returned from the Importer, Exporter, Testable,
 * Interpreter, Executable, and PlugIn interfaces.
 *
 * This class provides
 * metadata without ever instantiating the plug-in.
 */
class PlugInDescriptor : public SessionItem
{
public:
   /**
    * @copydoc PlugIn::getVersion()
    */
   virtual std::string getVersion() const = 0;

   /**
    * @copydoc PlugIn::isProduction()
    */
   virtual bool isProduction() const = 0;

   /**
    * @copydoc PlugIn::getCreator()
    */   
   virtual std::string getCreator() const = 0;

   /**
    * @copydoc PlugIn::getCopyright()
    */
   virtual std::string getCopyright() const = 0;

   /**
    * @copydoc PlugIn::getDependencyCopyright()
    */
   virtual std::map<std::string, std::string> getDependencyCopyright() const = 0;

   /**
    * @copydoc PlugIn::getDescription()
    */
   virtual std::string getDescription() const = 0;

   /**
    * @copydoc PlugIn::getShortDescription()
    */
   virtual std::string getShortDescription() const = 0;

   /**
    * @copydoc PlugIn::getType()
    */
   virtual std::string getType() const = 0;

   /**
    * @copydoc PlugIn::getSubtype()
    */
   virtual std::string getSubtype() const = 0;

   /**
    * @copydoc PlugIn::areMultipleInstancesAllowed()
    */
   virtual bool areMultipleInstancesAllowed() const = 0;

   /**
    * Queries the value of
    * Executable::isExecutedOnStartup() for Executable plug-ins.
    *
    * @return Returns the value provided by Executable::isExecutedOnStartup()
    * or false if the plug-in is not of type Executable.
    */
   virtual bool isExecutedOnStartup() const = 0;

   /**
    * Queries the value of
    * Executable::isDestroyedAfterExecute() for Executable plug-ins.
    * 
    * @return Returns the value provided by Executable::isDestroyedAfterExecute()
    * or true if the plug-in is not of type Executable.
    */
   virtual bool isDestroyedAfterExecute() const = 0;

   /**
    * Queries the value of
    * Executable::getMenuLocations() for Executable plug-ins.
    *
    * @return Returns the value provided by Executable::getMenuLocations()
    * or an empty vector if the plug-in is not of type Executable.
    */
   virtual const std::vector<std::string>& getMenuLocations() const = 0;

   /**
    * Queries the value of
    * Executable::hasAbort() for Executable plug-ins.
    * 
    * @return Returns the value provided by Executable::hasAbort()
    * or false if the plug-in is not of type Executable.
    */
   virtual bool hasAbort() const = 0;

   /**
    * Queries the value of
    * Executable::setBatch() for Executable plug-ins.
    * 
    * @return Returns true if Executable::setBatch()
    * will return true, false otherwise.  If
    * the plug-in is not of type Executable, false
    * will be returned.
    */
   virtual bool hasBatchSupport() const = 0;

   /**
    * Queries the value of
    * Executable::setInteractive() for Executable plug-ins.
    *
    * @return Returns true if Executable::setInteractive()
    * will return true, false otherwise.  If
    * the plug-in is not of type Executable, false
    * will be returned.
    */
   virtual bool hasInteractiveSupport() const = 0;

   /**
    * Queries the value of
    * Executable::hasWizardSupport() for Executable plug-ins.
    *
    * @return Returns the value provided by Executable::hasWizardSupport()
    * or false if the plug-in is not of type Executable.
    */
   virtual bool hasWizardSupport() const = 0;

   /**
    * Returns the batch input argument list for Executable plug-ins.
    *
    * The returned PlugInArgList is owned by this object,
    * so do not attempt to delete it.
    *
    * @return Returns the PlugInArgList that was returned
    * via the pArgList parameter of Executable::getInputSpecification()
    * after setBatch() was called on the plug-in instance. If
    * the plug-in is not of type Executable, NULL will be returned.
    */
   virtual const PlugInArgList* getBatchInputArgList() const = 0;

   /**
    * Returns the interactive input argument list for Executable plug-ins.
    *
    * The returned PlugInArgList is owned by this object,
    * so do not attempt to delete it.
    *
    * @return Returns the PlugInArgList that was returned
    * via the pArgList parameter of Executable::getInputSpecification()
    * after setInteractive() was called on the plug-in instance. If
    * the plug-in is not of type Executable, NULL will be returned.
    */
   virtual const PlugInArgList* getInteractiveInputArgList() const = 0;

   /**
    * Returns the batch output argument list for Executable plug-ins.
    *
    * The returned PlugInArgList is owned by this object,
    * so do not attempt to delete it.
    *
    * @return Returns the PlugInArgList that was returned
    * via the pArgList parameter of Executable::getOutputSpecification()
    * after setBatch() was called on the plug-in instance. If
    * the plug-in is not of type Executable, NULL will be returned.
    */
   virtual const PlugInArgList* getBatchOutputArgList() const = 0;

   /**
    * Returns the interactive output argument list for Executable plug-ins.
    *
    * The returned PlugInArgList is owned by this object,
    * so do not attempt to delete it.
    *
    * @return Returns the PlugInArgList that was returned
    * via the pArgList parameter of Executable::getOutputSpecification()
    * after setInteractive() was called on the plug-in instance. If
    * the plug-in is not of type Executable, NULL will be returned.
    */
   virtual const PlugInArgList* getInteractiveOutputArgList() const = 0;

   /**
    * Returns the file extensions supported by this plug-in.
    *
    * If the plug-in is of type Importer, then the returned
    * value will be Importer::getDefaultExtensions().  If the
    * plug-in is of type Exporter, then the returned value
    * will be Exporter::getDefaultExtensions().  If the plug-in
    * is of type Interpreter, then the returned value will be
    * Interpreter::getFileExtensions().  If the plug-in is 
    * none of these types, then an empty string will be
    * returned.
    *
    * @return The file extensions supported by this plug-in.
    */
   virtual std::string getFileExtensions() const = 0;

   /**
    * Queries whether the plug-in is Testable.
    *
    * @return Returns true if the plug-in instance could
    * be successfully cast to a Testable*, false
    * otherwise.
    */
   virtual bool isTestable() const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~PlugInDescriptor() {}
};

#endif
