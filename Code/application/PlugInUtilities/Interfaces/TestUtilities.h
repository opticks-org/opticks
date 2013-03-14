/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TESTUTILITIES_H
#define TESTUTILITIES_H

#include <iostream>
#include <string>

/**
 * Evaluates an expression and reports errors to an output stream.
 *
 * @param   expression
 *          The conditional source code expression to check.  It must evaluate
 *          to \c true or \c false.
 * @param   output
 *          The output stream in which to report an error.  An error is logged
 *          only if the given expression evaluates to \c false.
 *
 * @see     isseas()
 */
#define testAssert(expression, output) \
   ((expression) ? true : TestUtilities::assertionLogger(output, #expression, __FILE__, __LINE__))

/**
 * Evaluates an expression and updates a success flag.
 *
 * This macro first evaluates a success flag that must be named \em success.
 * If the success flag evaluates to \c false, then this macro does nothing.  If
 * the success flag evaluates to \c true, the given expression is evaluated
 * using the testAssert() macro and the success flag is updated to include the
 * evaluation result.
 *
 * isseas = <b>i</b>f (<b>s</b>uccess) <b>s</b>uccess and-<b>e</b>quals <b>a</b>ssert to <b>s</b>tream
 *
 * @param   expression
 *          The conditional source code expression to check.  It must evaluate
 *          to \c true or \c false.
 * @param   output
 *          The output stream in which to report an error.  An error is logged
 *          only if the given expression evaluates to \c false.
 *
 * @see     testAssert()
 */
#define isseas(expression, output) \
   success = success && testAssert(expression, output)

/**
 * Convenience functions for testing plug-ins and other code.
 *
 * This namespace contains functions that can be used by Testable plug-ins or
 * other code to implement test code to test plug-in functionality.
 *
 * The following related macros may also be helpful when writing test code:
 * - isseas()
 * - testAssert()
 */
namespace TestUtilities
{
   /**
    * Returns a directory path that can be used to import data for testing.
    *
    * @return  Returns the directory path specified in the import path
    *          configuration setting.
    *
    * @see     ConfigurationSettings::getSettingImportPath()
    */
   std::string getTestDataPath();

   /**
    * Reports errors to an output stream.
    *
    * @warning The assertion logger is used in conjunction with the testAssert()
    *          macro to report errors to an output stream, and should not be
    *          called directly.
    *
    * @param   output
    *          The output stream in which to report the error.
    * @param   expression
    *          The conditional source code expression causing the error.
    * @param   file
    *          The source file in which the error occurred.
    * @param   line
    *          The line in the source file where the error occurred.
    *
    * @return  Always returns false.
    */
   bool assertionLogger(std::ostream& output, const std::string& expression, const std::string& file, int line);
}

#endif
