/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTIONQWIDGETWRAPPER_H
#define OPTIONQWIDGETWRAPPER_H

#include "OptionShell.h"

#include <string>

/**
 * \ingroup ShellModule
 * This templated class provides a way to implement
 * both the Option interface and provide a QWidget
 * by writing a single class instead of two.
 * This templated class is used by writing a QWidget
 * subclass and then doing the following to register the
 * plug-in:
 *
 * @code
 *    //assume SampleOption is a subclass of QWidget and
 *    //the plug-in belongs to a module that was registered
 *    //using REGISTER_MODULE(SampleModule);
 *    REGISTER_PLUGIN(SampleModule, SampleOption, OptionQWidgetWrapper<SampleOption>());
 * @endcode
 *
 * The templated argument must meet the following requirements:
 *  <ul>
 *    <li>Provide a default constructor.</li>
 *    <li>Subclass QWidget.</li>
 *    <li>Provide a static getName() method. @see PlugIn::getName()</li>
 *    <li>Provide a static getOptionName() method. @see Option::getOptionName()</li>
 *    <li>Provide a static getDescription() method. @see PlugIn::getDescription()</li>
 *    <li>Provide a static getVersion() method. @see PlugIn::getVersion()</li>
 *    <li>Provide a static getCreator() method. @see PlugIn::getCreator()</li>
 *    <li>Provide a static getCopyright() method. @see PlugIn::getCopyright()</li>
 *    <li>Provide a static getShortDescription() method. @see PlugIn::getShortDescription()</li>
 *    <li>Provide a static isProduction() method. @see PlugIn::isProduction()</li>
 *    <li>Provide an applyChanges() method. @see Option::applyChanges()</li>
 *  </ul>
 */
template <typename OptionWidget>
class OptionQWidgetWrapper : public OptionShell
{
public:
   /**
    * Instantiates this option plug-in by
    * querying the templated class using
    * it's provided static methods.
    */
   OptionQWidgetWrapper()
   {
      PlugInShell::setName(OptionWidget::getName());
      setOptionName(OptionWidget::getOptionName());   
      setDescription(OptionWidget::getDescription());
      setVersion(OptionWidget::getVersion());
      setCreator(OptionWidget::getCreator());
      setCopyright(OptionWidget::getCopyright());
      setShortDescription(OptionWidget::getShortDescription());
      setProductionStatus(OptionWidget::isProduction());
      setDescriptorId(OptionWidget::getDescriptorId());
   }

   /**
    * Destroys this option plug-in.
    */
   ~OptionQWidgetWrapper()
   {
   }

   /**
    * Applies the changes by calling applyChanges()
    * on the templated class.
    */
   void applyChanges()
   {
      OptionWidget* pWidget = dynamic_cast<OptionWidget*>(getStoredWidget());
      if (pWidget == NULL)
      {
         return;
      }
      pWidget->applyChanges();
   }

protected:
   /**
    * Creates the option widget by instantiating the
    * templated class which must be a subclass of QWidget.
    */
   QWidget* createOptionsWidget()
   {
      return new OptionWidget();
   }
};

#endif
