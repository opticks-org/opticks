/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONSHELL_H
#define OPTIONSHELL_H

#include "Option.h"
#include "PlugInShell.h"

#include <string>

/**
 *  \ingroup ShellModule
 * A base class for plug-in shells or plug-in instances.
 *
 * This class provides a default implementation of the Option
 * plug-in interfaces.
 *
 * This class should generally NOT be subclassed directly,
 * instead try using OptionQWidgetWrapper.
 *
 * @see Option, OptionQWidgetWrapper
 */
class OptionShell : public Option, public PlugInShell
{
public:
   /**
    *  Creates the option plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::OptionType().
    *
    *  @see     getType()
    */
   OptionShell();

   /**
    *  Destroys the option plug-in.
    */
   virtual ~OptionShell();

   /**
    * @copydoc Option::getWidget()
    *
    * @default The default implementation will call
    * createOptionsWidget() once per instance of this
    * plug-in.
    */
   QWidget* getWidget();

   /**
    * @copydoc Option::getOptionName()
    *
    * @default The default implementation will return
    * value provided to setOptionName().
    */
   const std::string& getOptionName();

protected:
   /**
    * This method should be implemented in 
    * subclasses and can directly new the 
    * QWidget* and initialize it.  It
    * guaranteed to called at most once
    * per instance of this plug-in.
    */
   virtual QWidget* createOptionsWidget() = 0;

   /**
    * Returns the currently stored QWidget*.
    * If getWidget() has not yet been called
    * NULL will be returned.  Otherwise, the
    * value of createOptionsWidget() will
    * have been stored and that value is
    * returned.
    */
   virtual QWidget* getStoredWidget();

   /**
    * Sets the option name that will
    * be returned from getOptionName().
    *
    * @param name
    *        the option name.  Please see
    *        Option::getOptionName() for
    *        how the string is interpreted.
    *
    * @see Option::getOptionName()
    */
   void setOptionName(const std::string& name);

private:
   QWidget* mpOptionWidget;
   std::string mOptionName;
};

#endif
