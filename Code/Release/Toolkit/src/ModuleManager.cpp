
/**
 * Module Manager 
 *
 * The Module Manager is used to inform the main application about the
 * Plug-Ins within the Module.  It is also used to create, destroy, and provide
 * access to the Plug-Ins.  Plug-In developers edit this class to build
 * a Plug-In Module composed of their Plug-Ins. This is a singleton class.  
 * Only one instance of this class exists at a given time.  Use the instance() 
 * method to get a reference to the class.
 *
 */

#include "ModuleManager.h"

using namespace std;

/**
 * This is deprecated, so the value set does not currently matter.
 */
const char *ModuleManager::mspValidationKey = "None";

/**
 * Add any #include's required for your plug-in here.
 */

/**
 * Set this to the name of your module, ie. "Band Math"
 */
const char *ModuleManager::mspName = "";

/**
 * Set this to the version of your module, ie. "1.0"
 */
const char *ModuleManager::mspVersion = "";

/**
 * Set this to a description of your module, ie. "Use to perform math using the bands of a RasterElement"
 */
const char *ModuleManager::mspDescription = "";

/**
 * Set a unique identifier your your module, see SessionItem::getId() for more details
 */
const char *ModuleManager::mspUniqueId = "";

unsigned int ModuleManager::getTotalPlugIns()
{
   /**
    * Return the number of plug-ins contained within your dll or so here. 
    * NOTE: You can use the Service<> classes available in the application
    * at this point to read data files to dynamically determine how many plug-ins
    * your module has available.  
    */
   return 1;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
    PlugIn* pPlugIn = NULL;

    /**
     * This method will called with plugInNumbers between from 0 and the return value from getTotalPlugIns() - 1.
     * This method should construct the given subclass of the PlugIn interface and return the given pointer.
     * WARNING: The subclass of the PlugIn interface constructed for a given plugInNumber MUST be the same
     * for a single run of the application, it CANNOT change during an application run.
     * 
     * An example of implementing this is shown below:
     */

    switch (plugInNumber)
    {
        case 0:
           /**
            * Change this to create your plug-in instance.  Your plug-in must
            * be a subclass of the PlugIn interface.  Please see the *Shell classes
            * the .h files in Toolkit\Application\PlugInLib
            */
           pPlugIn = new myPlugIn; 
           break;
        default:
           break;
    }

   return pPlugIn;
}
