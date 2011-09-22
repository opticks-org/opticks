/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESQWIDGETWRAPPER_H
#define PROPERTIESQWIDGETWRAPPER_H

#include "NitfProperties.h"
#include "PlugInManagerServices.h"
#include "PlugInShell.h"

#include <memory>
#include <string>

namespace Nitf
{

/**
 *  \ingroup ShellModule
 *  A plug-in that both implements the Properties and Nitf::Properties interfaces and provides a
 *  QWidget.
 *
 *  This templated class is used by writing a QWidget subclass and
 *  then doing the following to register the plug-in:
 *
 * @code
 *    //assume SampleTreProperties is a subclass of QWidget and
 *    //the plug-in belongs to a module that was registered
 *    //using REGISTER_MODULE(SampleTreModule);
 *    REGISTER_PLUGIN(SampleTreModule, SampleTreProperties,
 *                    Nitf::PropertiesQWidgetWrapper<SampleTreProperties>());
 * @endcode
 *
 *  The templated argument must meet the following requirements:
 *  - Subclasses QWidget.
 *  - Provides a default constructor.
 *  - Provides a static getDescription() method.  @see PlugIn::getDescription()
 *  - Provides a static getVersion() method.  @see PlugIn::getVersion()
 *  - Provides a static getCreator() method.  @see PlugIn::getCreator()
 *  - Provides a static getCopyright() method.  @see PlugIn::getCopyright()
 *  - Provides a static getShortDescription() method.  @see PlugIn::getShortDescription()
 *  - Provices a static getTypeName() method. @see Nitf::Properties::getTypeName()
 *  - Provides a static isProduction() method.  @see PlugIn::isProduction()
 *  - Provides an applyChanges() method.  @see Properties::applyChanges()
 *  - Provides a canDisplayMetadata() method. @see Nitf::Properties::canDisplayMetadata()
 *
 * The plug-in name will be "Properties " + PropertiesWidget::getTypeName().
 */
template <typename PropertiesWidget>
class PropertiesQWidgetWrapper : public Nitf::Properties, public PlugInShell
{
public:
   /**
    *  Instantiates this properties plug-in by querying the templated class
    *  using its provided static methods.
    */
   PropertiesQWidgetWrapper() : mpWidget(NULL)
   {
      PlugInShell::setName(std::string("Properties ") + PropertiesWidget::getTypeName());
      setDescription(PropertiesWidget::getDescription());
      setVersion(PropertiesWidget::getVersion());
      setCreator(PropertiesWidget::getCreator());
      setCopyright(PropertiesWidget::getCopyright());
      setShortDescription(PropertiesWidget::getShortDescription());
      setProductionStatus(PropertiesWidget::isProduction());
      setDescriptorId(PropertiesWidget::getDescriptorId());
      setType(PlugInManagerServices::PropertiesType());
      setSubtype(Properties::SubType());
   }

   /**
    *  Destroys this properties plug-in.
    */
   virtual ~PropertiesQWidgetWrapper()
   {
   }

   /**
    *  Initializes the widget with a SessionItem by calling initialize() on the templated class.
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
    *  to be made or if the widget is for display purposes only.
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

   /**
    * @copydoc Nitf::Properties::getTypeName()
    */
   virtual std::string getTypeName() const
   {
      return PropertiesWidget::getTypeName();
   }

   /**
    * @copydoc Nitf::Properties::canDisplayMetadata()
    */
   virtual bool canDisplayMetadata(const DynamicObject& metadata)
   {
      const PropertiesWidget* pWidget = dynamic_cast<const PropertiesWidget*>(getWidget());
      if (pWidget != NULL)
      {
         return pWidget->canDisplayMetadata(metadata);
      }
      return false;
   }

   /**
    * @copydoc Properties::getWidget()
    */
   QWidget* getWidget()
   {
      if (mpWidget.get() == NULL)
      {
         mpWidget.reset(new PropertiesWidget());
      }
      return mpWidget.get();
   }

   /**
    * @copydoc Properties::getPropertiesName()
    */
   const std::string& getPropertiesName() const
   {
      return PropertiesWidget::getTypeName();
   }

private:
   std::auto_ptr<QWidget> mpWidget;
};

}

#endif
