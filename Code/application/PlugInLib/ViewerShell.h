/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef VIEWERSHELL_H
#define VIEWERSHELL_H

#include "AttachmentPtr.h"
#include "ExecutableShell.h"

/**
 *  \cond INTERNAL
 */
namespace boost
{
   class any;
}
/// \endcond
class QWidget;
class ApplicationServices;

/**
 *  \ingroup ShellModule
 *  A base class for plug-ins with modeless dialogs.
 *
 *  This class contains functionality to properly destroy modeless
 *  dialogs in a plug-in.  The default functionality of the class
 *  sets the destroy after execute flag to false so that the modeless
 *  dialog will not be destroyed when execute() successfully returns
 *  and the allow multiple instances flag to false so that only one
 *  modeless widget is available at any time.
 *
 *  The abort() method is used to register a PlugInCallback with
 *  DesktopServices to destroy the viewer widget.  Derived plug-in
 *  classes should therefore provide an implementation for the
 *  getWidget() method to return a pointer to the modeless dialog or
 *  other viewer widget.
 *
 *  Derived plug-in classes must call the abort() method when the
 *  executable should be destroyed.
 *  This typically occurs in an overridden QWidget::closeEvent()
 *  method in the modeless dialog or when the executable has detected an error
 *  and needs to close.
 *
 *  @see     ExecutableShell
 */
class ViewerShell : public ExecutableShell
{
public:
   /**
    *  Creates a viewer plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::ViewerType() and sets the plug-in
    *  to not be destroyed after execution.
    *
    *  @see     getType(), isDestroyedAfterExecute()
    */
   ViewerShell();

   /**
    *  Destroys the viewer plug-in.
    */
   ~ViewerShell();

   /**
    *  @copydoc Executable::getInputSpecification()
    *
    *  @default The default implementation does not set any args in the arg
    *           list and returns \b true.
    */
   bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default The default implementation does not set any args in the arg
    *           list and returns \b true.
    */
   bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  Destroys the viewer dialog or widget.
    *
    *  To perform specialized cleanup, this method should be overridden to
    *  perform the cleanup before calling the base class to destroy the viewer
    *  widget.
    *
    *  @default The default implementation of this method creates a
    *           PlugInCallback based on the widget returned from getWidget()
    *           and registers it with DesktopServices.  When the callback is
    *           executed, the widget is hidden and then destroyed.
    *
    *  @return  Returns \b true if the plug-in callback was successfully
    *           registered with DesktopServices for the viewer widget,
    *           otherwise returns \b false.
    *
    *  @see     PlugInCallback, DesktopServices::registerCallback()
    */
   bool abort();

   /**
    *  This slot is called when the ApplicationWindow closes.
    *
    *  The viewer widget is deleted so that proper cleanup order is
    *  preserved if the viewer is active during application shutdown.
    *
    *  @param subject
    *         This should be the DesktopServices pointer.
    *  @param signal
    *         DesktopServices::ApplicationWindowClosed
    *  @param data
    *         Invalid
    */
   void cleanupWidget(Subject& subject, const std::string& signal, const boost::any& data);

protected:
   /**
    *  Returns the viewer widget.
    *  A value of NULL implies that no viewer widget needs to be destroyed.
    *
    *  The viewer widget is typically a modeless dialog, but can be any QWidget.
    *  This method is called by the default implementation of the abort() method
    *  to destroy the widget.  A derived plug-in should implement this method to
    *  return a pointer to the widget so that it can be destroyed.
    *
    *  @return  A pointer to the viewer widget as a QWidget.
    *
    *  @see     abort()
    */
   virtual QWidget* getWidget() const = 0;

   AttachmentPtr<ApplicationServices> mpAppSvcsAttachment;

private:
   ViewerShell(const ViewerShell& rhs);
   ViewerShell& operator=(const ViewerShell& rhs);
};

#endif
