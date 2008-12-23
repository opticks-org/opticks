/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXECUTABLE_AGENT_H
#define EXECUTABLE_AGENT_H

#include <string>

class PlugIn;
class PlugInArgList;
class Progress;

/**
 *  This is a helper class that makes working with Executable
 *  plug-ins easier.  This class will manage the lifecycle of
 *  the Executable plug-in.  When this object is destroyed, the
 *  plug-in will be destroyed if it is set to be
 *  destroyed after execution and it is not a background plug-in.
 *  You should not create an instance of this class using
 *  ObjectFactory, but you should use ExecutableResource.
 *
 *  @warning If you do not call an overload of instantiate() before
 *  calling any other methods, a std::logic_error will be thrown.
 *  You cannot call instantiate() twice on the same instance
 *  or a std::logic_error will be thrown.
 *
 *  @see         ExecutableResource, Executable
 */
class ExecutableAgent
{
public:
   /**
    *  Creates an invalid object for delayed initialization of a ExecutableAgent.
    *
    *  Creates an invalid object where no plug-in is
    *  initially created.  The Executable can then be initialized later by calling
    *  setPlugIn().
    *
    *  @param   pProgress
    *           The progress object to pass into the plug-in.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the plug-in in batch mode
    *           or to \b false to execute the plug-in in interactive mode.
    *           Background plug-ins executed in interactive mode are
    *           automatically added to the Background Plug-In %Window.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(Progress* pProgress, bool batch) = 0;

   /**
    *  Creates a Executable to execute.
    *
    *  @param   plugInName
    *           The name of the Executable to create and execute.
    *  @param   menuCommand
    *           The menu command name from which the plug-in is executed.  The
    *           menu command name is set into an arg with "Menu Command" as its
    *           name and "string" as its type.  If this arg does not exist in
    *           the arg list of the given plug-in, this parameter is ignored.
    *           If an empty string is passed in, no arg value is set.
    *  @param   pProgress
    *           The progress object to pass into the plug-in.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the plug-in in batch mode
    *           or to \b false to execute the plug-in in interactive mode.
    *           Background plug-ins executed in interactive mode are
    *           automatically added to the Background Plug-In %Window.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(const std::string& plugInName, const std::string& menuCommand,
      Progress* pProgress, bool batch) = 0;

   /**
    *  Uses an Executable to execute.
    *
    *  @param   pPlugIn
    *           The plug-in to execute.  The agent assumes ownership of the
    *           given plug-in and will destroy it as necessary upon 
    *           destruction.
    *  @param   menuCommand
    *           The menu command name from which the plug-in is executed.  The
    *           menu command name is set into an arg with "Menu Command" as its
    *           name and "string" as its type.  If this arg does not exist in
    *           the arg list of the given plug-in, this parameter is ignored.
    *           If an empty string is passed in, no arg value is set.
    *  @param   pProgress
    *           The progress object to pass into the plug-in.  If \b NULL is
    *           passed in, a progress object is obtained by calling
    *           PlugInManagerServices::getProgress().
    *  @param   batch
    *           Set this value to \b true to execute the plug-in in batch mode
    *           or to \b false to execute the plug-in in interactive mode.
    *           Background plug-ins executed in interactive mode are
    *           automatically added to the Background Plug-In %Window.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method is called more than
    *           once on a instance.
    */
   virtual void instantiate(PlugIn* pPlugIn, const std::string& menuCommand,
      Progress* pProgress, bool batch) = 0;


   /**
    *  Sets the plug-in to execute.
    *
    *  @param   plugInName
    *           The name of the plug-in to execute.  If an empty string is
    *           passed in, the current plug-in is destroyed and no new plug-in
    *           is set, thereby creating an invalid object.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @see     setPlugIn(PlugIn*)
    */
   virtual void setPlugIn(const std::string& plugInName) = 0;

   /**
    *  Sets the plug-in to execute.
    *
    *  @param   pPlugIn
    *           The plug-in to execute.  The agent assumes ownership of the
    *           given plug-in and will destroy it as necessary upon 
    *           destruction.  If \b NULL is passed in, the current plug-in is
    *           destroyed and no new plug-in is set, thereby creating an
    *           invalid object.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @see     setPlugIn(const std::string&)
    */
   virtual void setPlugIn(PlugIn* pPlugIn) = 0;

   /**
    *  Creates a progress dialog when the agent is executed.
    *
    *  By default, if this method is not called, a progress dialog is not
    *  created.
    *
    *  This method does nothing if ApplicationServices::isInteractive() returns
    *  \c false.
    *
    *  @param   bCreate
    *           Set this parameter to \c true to create a progress dialog when
    *           the agent is executed.  A new progress dialog will then be
    *           created for each call to ExecutableAgent::execute() for this
    *           instance.
    *
    *  @throw   std::logic_error
    *           Thrown if ExecutableAgent::instantiate() method has not yet been
    *           called.
    *
    *  @see     ExecutableAgent::instantiate()
    */
   virtual void createProgressDialog(bool bCreate) = 0;

   /**
    *  Queries whether a progress dialog is created when the agent is executed.
    *
    *  @throw   std::logic_error
    *           Thrown if ExecutableAgent::instantiate() method has not yet been
    *           called.
    *
    *  @return  Returns \c true if a progress dialog is created when the agent
    *           is executed or \c false if the dialog is not created.  This
    *           method also returns \c false if createProgressDialog() has
    *           not been called or if ApplicationServices::isInteractive()
    *           returns \c false.
    */
   virtual bool isProgressDialogCreated() const = 0;

   /**
    *  Sets whether the input argument list will be automatically or manually
    *  populated.
    *
    *  If the input argument list is already populated, calling this function
    *  will not clear the list.
    *
    *  If this function is not called, the input argument list will be
    *  automatically populated.
    *
    *  @param   bAutoArg
    *           Set this parameter to \c true to automatically populate an
    *           input argument list or set it to \c false to populate the input
    *           argument list manually.
    */
   virtual void setAutoArg(bool bAutoArg) = 0;

   /**
    *  Gets whether the input argument list will be automatically or manually
    *  populated.
    *
    *  @return  Returns \c true if the input argument list is automatically
    *           populated or \c false if the input argument list is populated
    *           manually.
    */
   virtual bool getAutoArg() const = 0;

   /**
    *  Returns the progress object used by the plug-in.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The progress object used by the plug-in.  This is either the
    *           progress object passed in upon instantiation of the agent or
    *           the progress object obtained by calling
    *           PlugInManagerServices::getProgress().
    */
   virtual Progress* getProgress() const = 0;

   /**
    *  Executes the underlying plug-in.
    *
    *  This method first creates the input and output arg lists if necessary,
    *  populates the input arg list, and then executes the plug-in.  If a
    *  background plug-in was executed successfully, an item is automatically
    *  added to the Background Plug-In %Window.
    *
    *  @par Arg Values:
    *  The following input arg values are automatically populated unless the
    *  actual value in the arg has already been set and
    *  PlugInArg::isActualSet() returns \c true:
    *  - Executable::WindowArg() is populated with the current workspace window
    *    as returned by DesktopServices::getCurrentWorkspaceWindow().
    *  - Executable::ViewArg() is populated with the view contained in the
    *    current workspace window as returned by WorkspaceWindow::getView().
    *    If this view is not a kind of View as the arg type, the arg is
    *    populated with the active view contained in the current workspace
    *    window as returned by WorkspaceWindow::getActiveView().
    *  - Executable::LayerArg() is populated with the active layer in the view
    *    of the current workspace window as returned by
    *    SpatialDataView::getActiveLayer() or ProductView::getActiveLayer().
    *  - Executable::DataElementArg() is populated with the first DataElement
    *    found in the following search order:
    *      -# The element displayed by the primary raster layer in the view of
    *         the current workspace window as returned by
    *         LayerList::getPrimaryRasterElement().
    *      -# The element displayed by the active layer in the view of the
    *         current workspace window as returned by Layer::getDataElement().
    *      -# The first child element of the primary raster layer element in
    *         the view of the current workspace window.
    *
    *  @par
    *  If more than one of the args listed above exist and the actual value is
    *  set in the input arg list, one arg value will be used to get the value
    *  for another arg where appropriate instead of getting the value according
    *  to the above descriptions.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet been called on
    *           this instance.
    *
    *  @return  The success value returned by Executable::execute().  If the
    *           plug-in does not support the batch/interactive mode requested
    *           \c false will be returned and the plug-in will not be executed.
    */
   virtual bool execute() = 0;

   /**
    *  Aborts the underlying plug-in.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The success value returned by the plug-in's abort function.
    */
   virtual bool abort() = 0;

   /**
    *  Runs the tests specified in the Testable interface of the plug-in.
    *
    *  @param   pProgress
    *           The progress object to pass through the Testable interface.
    *  @param   output
    *           The output stream for the plug-in's test suite to use for
    *           reporting errors.
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return  The success value returned by Testable::runAllTests().  If
    *           the plug-in does not have a Testable interface, \b false is
    *           returned.
    */
   virtual bool runAllTests(Progress *pProgress, std::ostream &output) = 0;

   /**
    *  Access the input argument list.
    *
    *  This will ensure that the returned arg list has been built and has
    *  default values set.  It may or may not have actual values set, depending
    *  on when this function is called.
    *
    *  @warning The agent does not override actual values set into the input
    *           args.  Therefore, setting actual arg values on the returned
    *           PlugInArgList for objects that are explicitly set in the agent
    *           will cause the agent's values to not be passed into the
    *           plug-in.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return A reference to the input argument list.
    */
   virtual PlugInArgList &getInArgList() const = 0;

   /**
    *  Access the output argument list.
    *
    *  This will ensure that the returned arg list has been built and has
    *  default values set.  It may or may not have actual values set, depending
    *  on when this function is called.
    *
    *  @throw   std::logic_error
    *           Thrown if the instantiate() method has not yet
    *           been called on this instance.
    *
    *  @return A reference to the input argument list.
    */
   virtual PlugInArgList &getOutArgList() const = 0;

   /**
    * Returns the plug-in instance being
    * executed.
    *
    * @throw   std::logic_error
    *          Thrown if the instantiate() method has not yet
    *          been called on this instance.
    *
    * @return plug-in instance or NULL if not available.
    */
   virtual const PlugIn* getPlugIn() const = 0;

   /**
    * Returns the plug-in instance being
    * executed.
    *
    * @throw   std::logic_error
    *          Thrown if the instantiate() method has not yet
    *          been called on this instance.
    *
    * @return plug-in instance or NULL if not available.
    */
   virtual PlugIn* getPlugIn() = 0;

   /**
    * Releases the plug-in instance being
    * executed.  The plug-in instance will
    * no longer be destroyed when this
    * ExecutableAgent is destroyed.
    *
    * @throw   std::logic_error
    *          Thrown if the instantiate() method has not yet
    *          been called on this instance.
    *
    * @return  plug-in instance or NULL if not available.
    */
   virtual PlugIn* releasePlugIn() = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~ExecutableAgent() {}
};

#endif
