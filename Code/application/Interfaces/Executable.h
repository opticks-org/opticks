/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXECUTABLE_H
#define EXECUTABLE_H

class PlugInArgList;

#include <string>
#include <vector>

/**
 *  Provides a common interface for plug-in execution.
 *
 *  Defines a generic interface for all plug-ins that is used
 *  to query the plug-in for its input/ouput parameters and
 *  to execute it in a common way.
 */
class Executable
{
public:
   /**
    *  The name for a Progress argument.
    *
    *  Input arguments with this name will be automatically populated with a
    *  Progress pointer when the plug-in is executed with an ExecutableAgent.
    *  Arguments with this name should be of the type Progress.
    *
    *  @see     ExecutableAgent::execute()
    */
   static std::string ProgressArg() { return "Progress"; }

   /**
    *  The name for a menu command argument.
    *
    *  Input arguments with this name will be automatically populated with the
    *  menu command name from which the plug-in was executed when the plug-in
    *  is executed with an ExecutableAgent.  Arguments with this name should
    *  be of the type std::string.
    *
    *  @see     ExecutableAgent::execute()
    */
   static std::string MenuCommandArg() { return "Menu Command"; }

   /**
    *  The name for a Window argument.
    *
    *  Input arguments with this name will be automatically populated with a
    *  Window pointer when the plug-in is executed with an ExecutableAgent.
    *  Arguments with this name should be of the type Window or one of its
    *  subclasses.
    *  It should not be used by an Exporter or subclasses derived from Exporter
    *  to access the window associated with the exported item.
    *
    *  @see     ExecutableAgent::execute()
    */
   static std::string WindowArg() { return "Window"; }

   /**
    *  The name for a View argument.
    *
    *  Input arguments with this name will be automatically populated with a
    *  View pointer when the plug-in is executed with an ExecutableAgent.
    *  Arguments with this name should be of the type View or one of its
    *  subclasses.
    *  It should not be used by an Exporter or subclasses derived from Exporter
    *  to access the view associated with the exported item.
    *
    *  @see     ExecutableAgent::execute()
    */
   static std::string ViewArg() { return "View"; }

   /**
    *  The name for a Layer argument.
    *
    *  Input arguments with this name will be automatically populated with a
    *  Layer pointer when the plug-in is executed with an ExecutableAgent.
    *  Arguments with this name should be of the type Layer or one of its
    *  subclasses.
    *  It should not be used by an Exporter or classes derived from Exporter
    *  to access the layer associated with the exported item.
    *
    *  @see     ExecutableAgent::execute()
    */
   static std::string LayerArg() { return "Layer"; }

   /**
    *  The name for a DataElement argument.
    *
    *  Input arguments with this name will be automatically populated with a
    *  DataElement pointer when the plug-in is executed with an ExecutableAgent.
    *  Arguments with this name should be of the type DataElement or one of its
    *  subclasses.
    *  It should not be used by Exporter or classes derived from Exporter to access a 
    *  DataElement associated with the exported item.
    *
    *  @see     ExecutableAgent::execute()
    */
   static std::string DataElementArg() { return "Data Element"; }

   /**
    *  Queries whether the plug-in is automatically executed when the
    *  application starts.
    *
    *  @return  Returns \b true if the plug-in is automatically executed when
    *           the application is created; otherwise returns \b false.
    */
   virtual bool isExecutedOnStartup() const = 0;

   /**
    *  Queries whether the plug-in is automatically destroyed after it is
    *  successfully executed. Plug-ins which are not successfully executed are
    *  automatically destroyed by the application.
    *
    *  After a plug-in is successfully executed, it can be destroyed by the application
    *  or stay resident in memory.  A plug-in may need to exist after execution,
    *  such as those containing modeless GUI components that require user
    *  interaction or continually display data.
    *
    *  @return  Returns \b true if the plug-in is automatically destroyed after
    *           it is successfully executed; otherwise returns \b false.
    *
    *  @see     execute()
    */
   virtual bool isDestroyedAfterExecute() const = 0;

   /**
    *  Returns the menu locations and names of the commands from which the
    *  plug-in is executed.
    *
    *  @return  A vector of strings representing the menu locations and names
    *           of the commands that are used to execute the plug-in.<br><br>
    *           Each string is formatted with brackets ([,]) to specify a
    *           toolbar and a slash (/) to indicate submenus.  The toolbar name
    *           appears first in the string, followed by an optional slash, and
    *           then the menus, submenus and command name separated by slashes.
    *           If the optional slash appears following the toolbar name, this
    *           indicates that the submenus and command should be added to the
    *           default toolbar menu, which has the same name as the toolbar.
    *           If a slash does not follow the toolbar name, the menus and
    *           command are added directly to the toolbar.  If the string does
    *           not include a toolbar name, the menus and command are added to
    *           the main menu bar.  The string cannot end with a slash, and the
    *           name after the last slash indicates the command name.<br><br>
    *           Examples of the menu string include the following:
    *           <ul><li>[Geo]/Georeference</li>
    *           <li>&Tools/Flicker %Window</li></ul><br>
    *           An empty vector is returned if the plug-in should not be
    *           executed from the menus.
    */
   virtual const std::vector<std::string>& getMenuLocations() const = 0;

   /**
    *  Sets the plug-in to execute in batch mode.
    *
    *  This method is used to set the plug-in to execute in a
    *  non-GUI mode.
    *
    *  @return  Returns \b true if batch mode is supported and the plug-in was
    *           successfully set to execute in a non-GUI mode.  Returns
    *           \b false if the plug-in does not support batch mode, or if
    *           batch mode cannot currently be set on the plug-in.
    *
    *  @see     setInteractive()
    */
   virtual bool setBatch() = 0;

   /**
    *  Sets the plug-in to execute in interactive mode.
    *
    *  This method is used to set the plug-in to execute in a
    *  GUI mode.
    *
    *  @return  Returns \b true if interactive mode is supported and the
    *           plug-in was successfully set to execute in a GUI mode.  Returns
    *           \b false if the plug-in does not support interactive mode, or
    *           if interactive mode cannot currently be set on the plug-in.
    *
    *  @see     setBatch()
    */
   virtual bool setInteractive() = 0;

   /**
    *  Retrieves the plug-in input parameters.
    *
    *  This method queries the plug-in for its input parameters
    *  that are needed to execute properly.  The input arguments
    *  may be different in interactive mode and batch mode.
    *
    *  @param   pArgList
    *           A plug-in arg list pointer that is set to a created
    *           input argument list specifying the plug-in input
    *           parameters.  \b NULL is a valid pointer value if the
    *           plug-in does not require any input arguments.
    *
    *  @return  Returns \b true if the input parameter argument list was
    *           successfully created.  If the plug-in does not require input
    *           arguments, \b true is returned, but the given plug-in arg list
    *           pointer may be \b NULL.
    *
    *  @see     PlugInArgList
    */
   virtual bool getInputSpecification(PlugInArgList*& pArgList) = 0;

   /**
    *  Retrieves the plug-in output parameters.
    *
    *  This method queries the plug-in for its output parameters that are
    *  created during execution.  The output arguments may be different in
    *  interactive mode and batch mode.
    *
    *  @param   pArgList
    *           A plug-in arg list pointer that is set to a created
    *           output argument list specifying the plug-in output
    *           parameters.  \b NULL is a valid pointer value if the
    *           plug-in does not provide any output arguments.
    *
    *  @return  Returns \b true if the output parameter argument list was
    *           successfully created.  If the plug-in does not provide output
    *           arguments, \b true is returned, but the given plug-in arg list
    *           pointer may be \b NULL.
    *
    *  @see     PlugInArgList
    */
   virtual bool getOutputSpecification(PlugInArgList*& pArgList) = 0;

   /**
    *  Initializes the plug-in before execution.
    *
    *  This method is used by the plug-in manager to initialize the
    *  plug-in before it is executed.
    *
    *  @return  Returns \b true if the plug-in initialization was sucessful;
    *           otherwise returns \b false.
    */
   virtual bool initialize() = 0;

   /**
    *  Executes the plug-in.
    *
    *  @param   pInArgList
    *           On input, \em pInArgList contains a complete input argument
    *           list for the plug-in.  The actual values are used as inputs
    *           when executing the plug-in.  Default values may be used if
    *           an actual value is not present.
    *  @param   pOutArgList
    *           On input, \em pOutArgList contains a complete output argument
    *           list for the plug-in, although actual values and default values
    *           will be ignored.  On return, the actual values in the argument
    *           list will be updated to include all output parameters defined
    *           by the plug-in.
    *
    *  @return  Returns \b true if the execution was successful.  Returns
    *           \b false if an error occurred or if the user cancelled the
    *           plug-in while in interactive mode.
    *
    *  @see     getInputSpecification(), getOutputSpecification()
    */
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList) = 0;

   /**
    *  Aborts the plug-in during execution.
    *
    *  This method may be called by the application to attempt to gracefully
    *  abort a plug-in.  Depending on the specific plug-in implementation of
    *  this method, it may simply initiate the abort process and return
    *  or it may attempt to complete the abort process before returning.
    *
    *  @return  Returns \b true if the plug-in was successfully aborted or
    *           transformed to an aborted state, which does not necessarily
    *           mean that the plug-in has completed aborting.  Returns
    *           \b false if the plug-in could not abort successfully or if the
    *           plug-in does not support aborting.
    *
    *  @see     hasAbort()
    */
   virtual bool abort() = 0;

   /**
    *  Queries whether the plug-in can be aborted.
    *
    *  This method may be called to determine if the plug-in can be aborted.
    *  This can be useful in GUI applications where the user may not be given
    *  the chance to cancel.
    *
    *  @return  Returns \b true if the plug-in supports the ability to abort,
    *           otherwise returns \b false.  Plug-ins that do not have
    *           significant processing or loops can also return \b false.
    */
   virtual bool hasAbort() = 0;

   /**
    *  Queries whether the plug-in performs background processing.
    *
    *  This method is called by the application after executing the plug-in to
    *  determine if the plug-in should be unloaded or should wait until
    *  processing is finished.  This method is called before
    *  isDestroyedAfterExecute() is called, so a return value of \b false does
    *  not necessarily mean that the plug-in will be destroyed.
    *
    *  @return  Returns \b true if the plug-in performs background processing.
    *           Destruction and possible unloading of the plug-in will be
    *           delayed until the plug-in indicates that background processing
    *           has ended.  Returns \b false if the plug-in does not support
    *           background processing or is not performing background
    *           processing during this instance.
    *
    *  @see     DesktopServices::registerCallback()
    */
   virtual bool isBackground() const = 0;

   /**
    * Queries whether the plug-in supports being added as a wizard item.
    *
    * This method is called by the application to determine if the user
    * should be able to add this plug-in as a wizard item in the wizard
    * builder.
    *
    * @return Returns \b true if the user should be able to add this
    *         plug-in to the wizard builder.  Returns \b false if
    *         the user should NOT be able to add this plug-in to
    *         the wizard builder.
    */
   virtual bool hasWizardSupport() const = 0;

protected:
   /**
    * This should be destroyed by calling PlugInManagerServices::destroyPlugIn.
    */
   virtual ~Executable() {}
};

#endif
