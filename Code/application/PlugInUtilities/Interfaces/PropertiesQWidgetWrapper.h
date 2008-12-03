/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESQWIDGETWRAPPER_H
#define PROPERTIESQWIDGETWRAPPER_H

#include "PropertiesShell.h"

#include <string>

/**
 *  A plug-in that both implements the Properties interface and provides a
 *  QWidget.
 *
 *  This templated class is used by writing a QWidget subclass and then in
 *  ModuleManager::getPlugIn() doing the following:
 *
 * @code
 * // Assume SampleProperties is a QWidget subclass
 * switch (plugInNumber)
 * {
 *    case 0:
 *       pPlugIn = static_cast<PlugIn*>(new PropertiesQWidgetWrapper<SampleProperties>());
 *       break;
 * }
 * @endcode
 *
 *  The templated argument must meet the following requirements:
 *  - Subclasses QWidget.
 *  - Provides a default constructor.
 *  - Provides a static getName() method.  @see PlugIn::getName()
 *  - Provides a static getPropertiesName() method.  @see Properties::getPropertiesName()
 *  - Provides a static getDescription() method.  @see PlugIn::getDescription()
 *  - Provides a static getVersion() method.  @see PlugIn::getVersion()
 *  - Provides a static getCreator() method.  @see PlugIn::getCreator()
 *  - Provides a static getCopyright() method.  @see PlugIn::getCopyright()
 *  - Provides a static getShortDescription() method.  @see PlugIn::getShortDescription()
 *  - Provides a static isProduction() method.  @see PlugIn::isProduction()
 *  - Provides a applyChanges() method.  @see Properties::applyChanges()
 */
template <typename PropertiesWidget>
class PropertiesQWidgetWrapper : public PropertiesShell
{
public:
   /**
    *  Instantiates this properties plug-in by querying the templated class
    *  using its provided static methods.
    */
   PropertiesQWidgetWrapper()
   {
      PlugInShell::setName(PropertiesWidget::getName());
      setPropertiesName(PropertiesWidget::getPropertiesName());
      setDescription(PropertiesWidget::getDescription());
      setVersion(PropertiesWidget::getVersion());
      setCreator(PropertiesWidget::getCreator());
      setCopyright(PropertiesWidget::getCopyright());
      setShortDescription(PropertiesWidget::getShortDescription());
      setProductionStatus(PropertiesWidget::isProduction());
      setDescriptorId(PropertiesWidget::getDescriptorId());
   }

   /**
    *  Destroys this properties plug-in.
    */
   ~PropertiesQWidgetWrapper()
   {
   }

   /**
    *  Initializes the widget by calling initialize() on the templated class.
    *
    *  @param   pItem
    *           The session item for which to initialize the widget's values.
    *
    *  @return  Returns \c true if the widget was successfully initialized from
    *           the given session item; otherwise returns \c false.  If this
    *           method returns \c false, the widget is not added to the
    *           properties dialog.
    */
   bool initialize(SessionItem* pItem)
   {
      PropertiesWidget* pWidget = dynamic_cast<PropertiesWidget*>(getWidget());
      if (pWidget != NULL)
      {
         return pWidget->initialize(pItem);
      }

      return false;
   }

   /**
    *  Applies the changes in the widget.
    *
    *  This method is called when the user clicks the OK or Apply buttons in
    *  the properties dialog.  It applies the changes in the widget by calling
    *  applyChanges() on the templated class.  The applyChanges() method of the
    *  templated class should return \c true if all changes were successfully
    *  applied to the object.  It should also return \c true if no updates need
    *  to be made.
    *
    *  @return  This method returns the value returned from applyChanges() of
    *           the templated class or \c NULL if the widget has not been
    *           created.  If \c false is returned, the widget will be activated
    *           in the properties dialog if necessary and the dialog will not
    *           be closed if the OK button was clicked.
    */
   bool applyChanges()
   {
      PropertiesWidget* pWidget = dynamic_cast<PropertiesWidget*>(getWidget());
      if (pWidget != NULL)
      {
         return pWidget->applyChanges();
      }

      return false;
   }

protected:
   /**
    *  Creates the properties widget.
    *
    *  This method creates the properties widget by directly instantiating the
    *  templated class, which must be a subclass of QWidget.
    */
   QWidget* createWidget()
   {
      return new PropertiesWidget();
   }
};

#endif
