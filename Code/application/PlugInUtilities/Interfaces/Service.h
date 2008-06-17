/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SERVICE_H
#define SERVICE_H

/**
 * This class simplifies use of Services classes.
 *
 * For example, the following code:
 * \code
 *   ModelServices* pModelServices = NULL;
 *   ModuleManager::instance()->getService()->queryInterface("ModelServices2", reinterpret_cast<void**>(&pModelServices));
 *   bool bHasElement = pModelServices->hasElementType("AoiElement");
 * \endcode
 *
 * Would instead be:
 * \code
 *   Service<ModelServices> pModelServices;
 *   bool bHasElement = pModelServices->hasElementType("AoiElement");
 * \endcode
 *
 * The following can also be done:
 * \code
 *   bool bHasElement = Service<ModelServices>()->hasElementType("AoiElement");
 * \endcode
 *
 * This templated class can ONLY be used with the defined template
 * specializations or a link-time error will occur. The specializations are:
 *   \li Service<AnimationServices>
 *   \li Service<ApplicationServices>
 *   \li Service<ConfigurationSettings>
 *   \li Service<DataVariantFactory>
 *   \li Service<DesktopServices>
 *   \li Service<MessageLogMgr>
 *   \li Service<ModelServices>
 *   \li Service<ObjectFactory>
 *   \li Service<PlugInManagerServices>
 *   \li Service<SessionManager>
 *   \li Service<SessionExplorer>
 *   \li Service<UtilityServices>
 */
template<class T>
class Service
{
public:
   /**
    * Provides direct access to the Service pointer.
    *
    * @return A pointer of type T. No implementation is provided in order to
    *         force specialization. Specialized methods need to ensure that
    *         NULL will never be returned.
    */
   T *get() const;

   /**
    * Allows the held Service pointer to be used with pointer indirection.
    *
    * @return A pointer of type T; for all template specializations,
    *         this is guaranteed to be non-null.
    */
   T *operator->() const { return get(); }
};

#endif
