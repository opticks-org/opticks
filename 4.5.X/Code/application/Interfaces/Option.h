/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OPTION_H
#define OPTION_H

class QWidget;

#include <string>

/**
 * Interface for option plug-ins.
 *
 * All option plug-ins will be displayed to the user when opening the 
 * application options dialog, ie. "Tools\Options".  This interface can be 
 * implemented by a plug-in to get their custom option widgets to appear
 * in the application options dialog. The application options dialog
 * is modal and will instantiate all option plug-ins when shown and will
 * destroy all option plug-ins when the dialog is closed.
 *
 * @see OptionShell, OptionQWidgetWrapper
 */
class Option
{
public:
   /**
    * Returns the name of the option.
    *
    * This will be displayed to the 
    * user in a tree view, allowing them to show the widget returned
    * by getWidget() for this option.  This string can include '/',
    * which will then create a group in the tree view.  For instance,
    * a returned string of "Windows/Workspace/Cube" would create a tree
    * heirarchy like the following:
    *   <ul><li>Windows</li>
    *     <ul><li>Workspace</li>
    *       <ul><li>Cube</li></ul>
    *     </ul>
    *   </ul>
    *
    * If the string needs to actually include a '/', it can be escaped by
    * using '//'.  When placing option plug-ins into a heirarchy please
    * attempt to put them into the already provided application heirarchy
    * where possible.  If not possible, please create a top-level node
    * with the name of the plug-in suite and place option plug-ins
    * under that node.
    *
    * @return Returns the name of the option.
    */
   virtual const std::string& getOptionName() = 0;

   /**
    * Returns the widget that will be displayed to the user.
    * 
    * The widget will only be displayed when the user
    * selects the option in the options dialog.
    * This widget should only be instantiated
    * once per instance of this plug-in.  This
    * widget will be owned by this plug-in and should
    * be destroyed when the plug-in is destroyed.
    * Please note, that the option dialog that this widget will
    * be displayed in is resizable, so ensure the Qt layout for
    * this widget resizes properly.
    *
    * @warning The QWidget* returned by this method should NEVER be
    * instantiated in the constructor of this plug-in.  When the
    * application is run in batch mode, it will attempt to
    * create this plug-in and if this plug-in attempts to create
    * a QWidget* in it's constructor, the application will crash.
    *
    * @return Returns the widget that will be displayed to the user.
    */
   virtual QWidget* getWidget() = 0;

   /**
    * Apply the changes that the user made in the QWidget.
    *
    * The values modified by the user while the QWidget
    * was displayed should be stored permanently.
    * This usually means storing the values in ConfigurationSettings,
    * but can include any method that will store the values such
    * that the values will be retained through a stop and start
    * of the application and the values should generally be associated
    * with the user running the application only.
    * The values modified by the user should NEVER be permanently
    * stored until this method is called.  This method is guaranteed
    * to only be called once per instance of the class.
    */
   virtual void applyChanges() = 0;

protected:
   /**
    *  Since the Option interface is usually used in conjunction with the
    *  PlugIn interface, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~Option() {}
};

#endif
