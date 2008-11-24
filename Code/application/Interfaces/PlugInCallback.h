/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINCALLBACK_H_
#define PLUGINCALLBACK_H_

class PlugIn;
class Progress;

class PlugInCallback
{
public:
   /**
    *  This is called when cleanup has finished in the core.
    */
   virtual void operator()() = 0;

   /**
    *  This blocks until the plug-in thread has finished.
    *
    *  Usually just a wrapper around BThread::ThreadWait().
    *
    *  @return  Returns \b true if successful, otherwise returns \b false.
    */
   virtual bool finish() = 0;

   /**
    *  Returns the Progress object for the plug-in that registered this
    *  callback.
    *
    *  @return  The Progress object for the registering plug-in.
    */
   virtual Progress* progress() const = 0;

   /**
    *  Returns a pointer to the plug-in that is registering this callback.
    *
    *  @return  A pointer to the plug-in that has registered this callback.
    *           \b NULL may be returned if the plug-in has been registered as a
    *           background executing plug-in.
    *
    *  @see     ViewerShell, DesktopServices::registerCallback()
    */
    virtual PlugIn* getPlugIn() const = 0;

    /**
     * This should be implemented in subclasses.
     */
    virtual ~PlugInCallback() {}
};

#endif
