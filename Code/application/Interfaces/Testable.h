/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TESTABLE_H
#define TESTABLE_H

#include <ostream>

class Progress;

/**
 *  A common testing interface for all plug-ins.
 *
 *  This class defines the generic testing interface to all plug-ins.  This
 *  interface is used to verify plug-ins in an operational environment as well
 *  as to run a full suite of automated tests.
 */
class Testable
{
public:
   /**
    *  Executes operational tests generically.
    *
    *  This method is used to run basic tests and to make sure the plug-in is
    *  operating properly.  Running this method on all plug-ins can be used to
    *  relatively quickly determine if the application has been installed
    *  properly and all plug-ins are operational.
    *
    *  @param   pProgress
    *           On input, \em pProgress provides an object for the plug-in to
    *           update the progress of the testing.  This allows the
    *           application to update the user with a progress dialog and to
    *           show current status.
    *  @param   failure
    *           On output, \em failure is an output stream in which the plug-in
    *           can record detailed formatted text about what went wrong in
    *           with the test.  This parameter should not be used to record
    *           general status information.  Nothing should be logged to this
    *           stream unless the tests failed and \b false is returned from
    *           the method.
    *
    *  @return  Returns \b true if the operational tests executed successfully;
    *           otherwise returns \b false.
    */
   virtual bool runOperationalTests(Progress* pProgress, std::ostream& failure) = 0;

   /**
    *  Executes the full suite of tests.
    *
    *  This method is used to run a full set of tests and to make sure the
    *  plug-in is operating properly.  This method can be called on all
    *  plug-ins to run a full set of tests and to determine if the current
    *  build works completely.  This method is used more for automated testing
    *  during the development and formal testing process.
    *
    *  @param   pProgress
    *           On input, \em pProgress provides an object for the plug-in to
    *           update the progress of the testing.  This allows the
    *           application to update the user with a progress dialog and to
    *           show current status.
    *  @param   failure
    *           On output, \em failure is an output stream in which the plug-in
    *           can record detailed formatted text about what went wrong in
    *           with the test.  This parameter should not be used to record
    *           general status information.  Nothing should be logged to this
    *           stream unless the tests failed and \b false is returned from
    *           the method.
    *
    *  @return  Returns \b true if the all tests executed successfully;
    *           otherwise returns \b false.
    */
   virtual bool runAllTests(Progress* pProgress, std::ostream& failure) = 0;

protected:
   /**
    * This should be destroyed by calling PlugInManagerServices::destroyPlugIn.
    */
   virtual ~Testable() {}
};

#endif
