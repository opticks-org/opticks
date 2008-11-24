/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONFIGURATIONSETTINGS_H
#define CONFIGURATIONSETTINGS_H

#include "DataVariant.h"
#include "Filename.h"
#include "Subject.h"
#include "Service.h"
#include "TypesFile.h"

#include <map>
#include <string>

class DynamicObject;
class Filename;

/**
 * This macro creates 4 static functions that
 * will provide get/set, has and getName functions for
 * a given setting in ConfigurationSettings.  These
 * methods should be used because they create type-safe
 * get/set functions for the given setting.  This macro
 * should be used for non-pointer types, the SETTING_PTR() macro
 * should be used for pointer types.
 *
 * Please see \ref settingsmacros for more details
 *
 * @param settingname
 *        the name of the setting.  The three generated methods will
 *        be getSetting[settingname], setSetting[settingname] and
 *        hasSetting[settingname]
 * @param classname
 *        this will be used to namespace the setting within
 *        ConfigurationSettings, ie. the key used to store
 *        the value in ConfigurationSettings will be 
 *        classname + "/" + settingname.
 * @param type
 *        this should be the C++ type used to store the setting,
 *        ie. unsigned int, bool.  This version of the macro
 *        should only be used with non-pointer types (ie. value types)
 * @param errorDefault
 *        The getSetting method generated by this macro must
 *        always return a value. If the setting does not
 *        exist in ConfigurationSettings, the value
 *        provided here will be returned by the getSetting
 *        method.  If the value provided here must be
 *        returned a verification error will be raised by
 *        the getSetting function.
 *
 */
#define SETTING(settingname,classname,type,errorDefault) \
   /**
    * Returns the current value for this setting.  If this
    * setting does not exist in ConfigurationSettings, a
    * verification error will be logged to the message log
    * and potentially a verification error message box
    * will be displayed to the user.
    *
    * Please see \ref settingsmacros for more details
    * 
    * \return the current value for this setting.
    */ \
   static type getSetting##settingname() \
   { \
      Service<ConfigurationSettings> pSettings; \
      return dv_cast_with_verification< type >(pSettings->getSetting(getSetting##settingname##Key()), errorDefault); \
   } \
   /**
    * Returns true if this setting exists and has a 
    * value in ConfigurationSettings.
    *
    * Please see \ref settingsmacros for more details
    *
    * \return true if this setting exists, false otherwise.
    */ \
   static bool hasSetting##settingname() \
   { \
      Service<ConfigurationSettings> pSettings; \
      const DataVariant& dv = pSettings->getSetting(getSetting##settingname##Key()); \
      if (dv_cast< type >(&dv) != NULL) \
      { \
         return true; \
      } \
      return false; \
   } \
   /**
    * Changes the current value of this setting to the new value.
    *
    * Please see \ref settingsmacros for more details
    *
    * \param newValue
    *        the new value for this setting.
    * \param setIfSame
    *        If true, the value will be set into ConfigurationSettings
    *        even if the newValue and existing value are the same.  This
    *        has the side-effect of making the value a per-user setting
    *        and stored in the user's configuration file.  If false, this
    *        value will only be set if the newValue and existing value
    *        are different.  Therefore the value will only be stored in 
    *        the user's configuration file if the newValue is unique
    *        to the user.
    */ \
   static void setSetting##settingname(type newValue, bool setIfSame = false) \
   { \
      Service<ConfigurationSettings> pSettings; \
      pSettings->setSetting(getSetting##settingname##Key(), newValue, setIfSame); \
   } \
   /**
    * Returns the key for this setting that
    * could be passed to ConfigurationSettings::getSetting()
    * or ConfigurationSettings::setSetting().
    *
    * Please see \ref settingsmacros for more details
    *
    * @return the key used for this setting.
    */ \
    static std::string getSetting##settingname##Key() \
   { \
      std::string keyValue(#classname "/" #settingname); \
      return keyValue; \
   } \

/**
 * This macro creates 4 static functions that
 * will provide get/set, has and getName functions for
 * a given setting in ConfigurationSettings.  These
 * methods should be used because they create type-safe
 * get/set functions for the given setting.  This macro
 * should be used for pointer types, the SETTING() macro
 * should be used for non-pointer types.
 *
 * Please see \ref settingsmacros for more details
 *
 * @param settingname
 *        the name of the setting.  The three generated methods will
 *        be getSetting[settingname], setSetting[settingname] and
 *        hasSetting[settingname]
 * @param classname
 *        this will be used to namespace the setting within
 *        ConfigurationSettings, ie. the key used to store
 *        the value in ConfigurationSettings will be 
 *        classname + "/" + settingname.
 * @param type
 *        this should be the C++ type used to store the setting,
 *        ie. DynamicObject, Filename (ie. without the *).
 *        The macro will add the * where required.
 */
#define SETTING_PTR(settingname,classname,type) \
   /**
    * Returns the current value for this setting.  If this
    * setting does not exist in ConfigurationSettings, a
    * NULL value will be returned.
    *
    * Please see \ref settingsmacros for more details
    * 
    * \return the current value for this setting or NULL if not found.
    */ \
   static const type* getSetting##settingname() \
   { \
      Service<ConfigurationSettings> pSettings; \
      return dv_cast< type >(&pSettings->getSetting(getSetting##settingname##Key())); \
   } \
   /**
    * Returns true if this setting exists in ConfigurationSettings.
    * It may still be NULL however.
    *
    * Please see \ref settingsmacros for more details
    *
    * \return true if this setting exists, false otherwise.
    */ \
   static bool hasSetting##settingname() \
   { \
      Service<ConfigurationSettings> pSettings; \
      const DataVariant& dv = pSettings->getSetting(getSetting##settingname##Key()); \
      if (dv_cast< type >(&dv) != NULL) \
      { \
         return true; \
      } \
      return false; \
   } \
   /**
    * Changes the current value of this setting to the new value.
    *
    * Please see \ref settingsmacros for more details
    *
    * \param pNewValue
    *        the new value for this setting.  If NULL, the value will not be set.
    * \param setIfSame
    *        If true, the value will be set into ConfigurationSettings
    *        even if the pNewValue and existing value are the same.  This
    *        has the side-effect of making the value a per-user setting
    *        and stored in the user's configuration file.  If false, this
    *        value will only be set if the pNewValue and existing value
    *        are different.  Therefore the value will only be stored in 
    *        the user's configuration file if the pNewValue is unique
    *        to the user.
    */ \
   static void setSetting##settingname(type* pNewValue, bool setIfSame = false) \
   { \
      Service<ConfigurationSettings> pSettings; \
      if (pNewValue != NULL) \
      { \
         pSettings->setSetting(getSetting##settingname##Key(), *pNewValue, setIfSame); \
      } \
   } \
   /**
    * Returns the key for this setting that
    * could be passed to ConfigurationSettings::getSetting()
    * or ConfigurationSettings::setSetting().
    *
    * Please see \ref settingsmacros for more details
    *
    * @return the key used for this setting.
    */ \
    static std::string getSetting##settingname##Key() \
   { \
      std::string keyValue(#classname "/" #settingname); \
      return keyValue; \
   }

/**
 * This macro creates 4 static functions that
 * will provide get/set, has and getName functions for
 * a given setting in ConfigurationSettings.  These
 * methods should be used because they create type-safe
 * get/set functions for the given setting.
 * This macro should be used instead of SETTING() if
 * and additional string needs to be provided at runtime
 * to make a unique key for the setting.  For instance, if
 * there should be a different setting value for each plug-in, then
 * this macro will generate functions that take the plug-in name
 * as an argument.
 * This macro should be used for non-pointer types, the CUSTOM_SETTING_PTR() macro
 * should be used for pointer types.
 *
 * Please see \ref settingsmacros for more details
 *
 * @param settingname
 *        the name of the setting.  The three generated methods will
 *        be getSetting[settingname], setSetting[settingname] and
 *        hasSetting[settingname]
 * @param classname
 *        this will be used to namespace the setting within
 *        ConfigurationSettings, ie. the key used to store
 *        the value in ConfigurationSettings will be 
 *        classname + "/" + namespaceStr + "/" + settingname.
 *        namespaceStr is an argument to the getSetting/setSetting
 *        and hasSetting functions generated by this macro.
 * @param type
 *        this should be the C++ type used to store the setting,
 *        ie. unsigned int, bool.  This version of the macro
 *        should only be used with non-pointer types (ie. value types)
 * @param errorDefault
 *        The getSetting method generated by this macro must
 *        always return a value. If the setting does not
 *        exist in ConfigurationSettings, the value
 *        provided here will be returned by the getSetting
 *        method.  If the value provided here must be
 *        returned a verification error will be raised by
 *        the getSetting function.
 */
#define CUSTOM_SETTING(settingname,classname,type,errorDefault) \
   /**
    * Returns the current value for this setting.  If this
    * setting does not exist in ConfigurationSettings, a
    * verification error will be logged to the message log
    * and potentially a verification error message box
    * will be displayed to the user.
    *
    * Please see \ref settingsmacros for more details
    *
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    * 
    * \return the current value for this setting.
    */ \
   static type getSetting##settingname(const std::string& namespaceStr ) \
   { \
      Service<ConfigurationSettings> pSettings; \
      return dv_cast_with_verification< type >(pSettings->getSetting(getSetting##settingname##Key(namespaceStr)), errorDefault); \
   } \
   /**
    * Returns true if this setting exists and has a 
    * value in ConfigurationSettings.
    *
    * Please see \ref settingsmacros for more details
    *
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    *
    * \return true if this setting exists, false otherwise.
    */ \
   static bool hasSetting##settingname(const std::string& namespaceStr ) \
   { \
      Service<ConfigurationSettings> pSettings; \
      const DataVariant& dv = pSettings->getSetting(getSetting##settingname##Key(namespaceStr)); \
      if (dv_cast< type >(&dv) != NULL) \
      { \
         return true; \
      } \
      return false; \
   } \
   /**
    * Changes the current value of this setting to the new value.
    *
    * Please see \ref settingsmacros for more details
    *
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    * \param newValue
    *        the new value for this setting.
    * \param setIfSame
    *        If true, the value will be set into ConfigurationSettings
    *        even if the newValue and existing value are the same.  This
    *        has the side-effect of making the value a per-user setting
    *        and stored in the user's configuration file.  If false, this
    *        value will only be set if the newValue and existing value
    *        are different.  Therefore the value will only be stored in 
    *        the user's configuration file if the newValue is unique
    *        to the user.
    */ \
   static void setSetting##settingname(const std::string& namespaceStr, type newValue, bool setIfSame = false) \
   { \
      Service<ConfigurationSettings> pSettings; \
      pSettings->setSetting(getSetting##settingname##Key(namespaceStr), newValue, setIfSame); \
   } \
   /**
    * Returns the key for this setting that
    * could be passed to ConfigurationSettings::getSetting()
    * or ConfigurationSettings::setSetting().
    *
    * Please see \ref settingsmacros for more details
    *
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    *
    * @return the key used for this setting.
    */ \
    static std::string getSetting##settingname##Key(const std::string& namespaceStr) \
   { \
      std::string keyValue(#classname "/" + namespaceStr + "/" #settingname); \
      return keyValue; \
   }

/**
 * This macro creates 4 static functions that
 * will provide get/set, has and getName functions for
 * a given setting in ConfigurationSettings.  These
 * methods should be used because they create type-safe
 * get/set functions for the given setting. 
 * This macro should be used instead of SETTING_PTR() if
 * and additional string needs to be provided at runtime
 * to make a unique key for the setting.  For instance, if
 * there should be a different setting value for each plug-in, then
 * this macro will generate functions that take the plug-in name
 * as an argument.
 * This macro should be used for pointer types, the CUSTOM_SETTING() macro
 * should be used for non-pointer types.
 *
 * Please see \ref settingsmacros for more details
 *
 * @param settingname
 *        the name of the setting.  The three generated methods will
 *        be getSetting[settingname], setSetting[settingname] and
 *        hasSetting[settingname]
 * @param classname
 *        this will be used to namespace the setting within
 *        ConfigurationSettings, ie. the key used to store
 *        the value in ConfigurationSettings will be 
 *        classname + "/" + namespaceStr + "/" + settingname.
 *        namespaceStr is an argument to the getSetting/setSetting
 *        and hasSetting functions generated by this macro.
 * @param type
 *        this should be the C++ type used to store the setting,
 *        ie. DynamicObject, Filename (ie. without the *).
 *        The macro will add the * where required.
 */
#define CUSTOM_SETTING_PTR(settingname,classname,type) \
   /**
    * Returns the current value for this setting.  If this
    * setting does not exist in ConfigurationSettings, a
    * NULL value will be returned.
    *
    * Please see \ref settingsmacros for more details
    * 
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    *
    * \return the current value for this setting or NULL if not found.
    */ \
   static const type* getSetting##settingname(const std::string& namespaceStr) \
   { \
      Service<ConfigurationSettings> pSettings; \
      return dv_cast< type >(&pSettings->getSetting(getSetting##settingname##Key(namespaceStr))); \
   } \
   /**
    * Returns true if this setting exists in ConfigurationSettings.
    * It may still be NULL however.
    *
    * Please see \ref settingsmacros for more details
    *
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    *
    * \return true if this setting exists, false otherwise.
    */ \
   static bool hasSetting##settingname(const std::string& namespaceStr ) \
   { \
      Service<ConfigurationSettings> pSettings; \
      const DataVariant& dv = pSettings->getSetting(getSetting##settingname##Key(namespaceStr)); \
      if (dv_cast< type >(&dv) != NULL) \
      { \
         return true; \
      } \
      return false; \
   } \
   /**
    * Changes the current value of this setting to the new value.
    *
    * Please see \ref settingsmacros for more details
    *
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    * \param pNewValue
    *        the new value for this setting.  If NULL, the value will not be set.
    * \param setIfSame
    *        If true, the value will be set into ConfigurationSettings
    *        even if the pNewValue and existing value are the same.  This
    *        has the side-effect of making the value a per-user setting
    *        and stored in the user's configuration file.  If false, this
    *        value will only be set if the pNewValue and existing value
    *        are different.  Therefore the value will only be stored in 
    *        the user's configuration file if the pNewValue is unique
    *        to the user.
    */ \
   static void setSetting##settingname(const std::string& namespaceStr, type* pNewValue, bool setIfSame = false) \
   { \
      Service<ConfigurationSettings> pSettings; \
      if (pNewValue != NULL) \
      { \
         pSettings->setSetting(getSetting##settingname##Key(namespaceStr), *pNewValue, setIfSame); \
      } \
   } \
   /**
    * Returns the key for this setting that
    * could be passed to ConfigurationSettings::getSetting()
    * or ConfigurationSettings::setSetting().
    *
    * Please see \ref settingsmacros for more details
    *
    * \param namespaceStr
    *        This string will be used to create a unique
    *        identifier for this particular setting so
    *        that it can be stored and retrieved from ConfigurationSettings.
    *
    * @return the key used for this setting.
    */ \
    static std::string getSetting##settingname##Key(const std::string& namespaceStr) \
   { \
      std::string keyValue(#classname "/" + namespaceStr + "/" #settingname); \
      return keyValue; \
   }
   
/**
 *  Contains settings specific to the user and application run that
 *  will be persisted between runs of the application.
 *
 *  The configuration settings allow data values to be stored per-user and
 *  during a application run.  Settings which are saved per-user and persist
 *  between application runs are called per-user settings.  Settings which
 *  only exist during one application run are called per-session settings.
 *
 *  These settings are initialized on application startup.  This is done by
 *  locating any .cfg files contained in getHome()/DefaultSettings and the
 *  directory defined by the "/defaultDir" command-line argument.  These .cfg
 *  files must follow the naming convention of "[LoadOrder]-[Name].cfg" where
 *  LoadOrder is an unsigned int and Name can be any text recognized by the file
 *  system. These .cfg's are then parsed according to the LoadOrder, with 1 reserved
 *  for the application and being the first .cfg that will be parsed.  After
 *  these files are parsed, there is a .cfg file for the user that is running
 *  the application that will be parsed and created as per-user settings.
 *
 *  The configuration settings can also be used to allow the Studio to create
 *  PlugInArg objects prior to plug-in execution.  If the plug-in args cannot be
 *  defined the configuration setting are queried for specific values using the
 *  plug-in name for the Setting group name.
 *
 *  The configuration settings interface has been extended with addition of the new
 *  extension interface ConfigurationSettingsExt1. 
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setSetting(), setSessionSetting,
 *    deleteUserSetting, deleteSessionSetting.
 */
class ConfigurationSettings : public Subject
{
public:
   /**
    *  Emitted with any<std::string> when setSetting or setSessionSetting is called.
    *  The string will be the key of the setting that was modified.
    */
   SIGNAL_METHOD(ConfigurationSettings, SettingModified);
   /**
    *  Emitted with no data just prior to save of the configuration settings
    *  to disk.
    */
   SIGNAL_METHOD(ConfigurationSettings, AboutToSave);

   SETTING_PTR(ExportPath, FileLocations, Filename)
   SETTING_PTR(ImportPath, FileLocations, Filename)
   SETTING_PTR(SaveOpenSessionPath, FileLocations, Filename)
   SETTING_PTR(InternalPath, FileLocations, Filename)
   SETTING(PathBookmarks, FileLocations, std::vector<Filename*>, std::vector<Filename*>())
   SETTING_PTR(MessageLogPath, FileLocations, Filename)
   SETTING(NumberOfMruFiles, General, unsigned int, 3)
   SETTING_PTR(SupportFilesPath, FileLocations, Filename)
   SETTING(ShowStatusBarCubeValue, StatusBar, bool, true)
   SETTING(ShowStatusBarElevationValue, StatusBar, bool, true)
   SETTING(ShowStatusBarGeoCoords, StatusBar, bool, true)
   SETTING(ShowStatusBarPixelCoords, StatusBar, bool, true)
   SETTING(ShowStatusBarResultValue, StatusBar, bool, true)
   SETTING(ShowStatusBarRotationValue, StatusBar, bool, true)
   SETTING_PTR(TempPath, FileLocations, Filename)
   SETTING(ThreadCount, Edit, unsigned int, 1)
   SETTING(UndoBufferSize, Edit, unsigned int, 10)
   SETTING_PTR(WizardPath, FileLocations, Filename)
   SETTING_PTR(JvmLoaderLibraryPath, FileLocations, Filename)
   CUSTOM_SETTING_PTR(PluginWorkingDirectory, FileLocations, Filename)

   /**
    * Gets the root directory for the main application.
    *
    * This method returns the fully qualified path for the base 
    * of the directory tree associated with the main application.
    *
    * @return   Base folder for the main application.
    */
   virtual std::string getHome() const = 0;

   /**
    * Gets the plug-in path.  This is the directory
    * where plug-ins must be placed if they
    * are to be loaded by the application.
    * 
    * @return   The plug-in path.
    */
   virtual std::string getPlugInPath() const = 0;

   /**
    * Gets the application creator.
    *
    * @return   The application creator.
    */
   virtual std::string getCreator() const = 0;

   /**
    * Gets the application name.
    *
    * @return   The application name.
    */
   virtual std::string getProduct() const = 0;

   /**
    * Gets the application version.
    *
    * @return   The application version.
    */
   virtual std::string getVersion() const = 0;

   /**
    * Gets the build revision of the application.
    *
    * @return The build revision.
    */
   virtual std::string getBuildRevision() const = 0;

   /**
    * Gets the username of the user currently running the application.
    *
    * @return   The username of the current user.
    */
   virtual std::string getUserName() const = 0;

   /**
    * Gets the operating system name.
    *
    * Returns the general operating system name that the
    * application was compiled against. (ie. "Solaris" or "Windows")
    * This name should only be used within the context of
    * the application and is not intended to be passed to any
    * system api's or used to conditionally execute code, see
    * AppConfig.h for that.  This name is also not
    * guaranteed to match any similiar operating system
    * name returned by using system calls.
    *
    * @return The operating system name.
    */
   virtual std::string getOperatingSystemName() const = 0;

   /**
    * Gets the architecture name.
    *
    * Returns the general architecture name that the
    * application was compiled to work with. (ie. "x86-64", "Sparc64")
    * This name should only be used within the context of
    * the application and is not intended to be passed to any
    * system api's or used to conditionally execute code, see
    * AppConfig.h for that.  This name is also not
    * guaranteed to match any similiar architecture 
    * name returned by using system calls.
    *
    * @return The architecture name.
    */
   virtual std::string getArchitectureName() const = 0;

   /**
    * Gets the production status.
    *
    * @return   True if the application and all plug-ins are production.
    */
   virtual bool isProductionRelease() const = 0;

   /**
    * Gets the release type.
    *
    * @return  An enum describing the release type.
    */
   virtual ReleaseType getReleaseType() const = 0;

   /**
    * Sets the given setting.
    *
    * This method sets the given setting.  If the setting
    * already exists, the data is simply replaced.  If the
    * setting does not exist, it is created.  This
    * setting will be saved into the user's configuration file
    * during application shutdown
    * meaning the particular value set for this setting will be
    * a per-user value.  If the given setting was previously set
    * with setSessionSetting(), it will no longer be a session setting
    * and will become a per-user setting.
    *
    * @param   key
    *          The name of the setting.  This key
    *          can have '/' in it, in which it will behave like DynamicObject::setAttributeByPath.
    * @param   var
    *          The value of the setting. If this is invalid, no setting will 
    *          be added or modified.
    * @param   setIfSame
    *          If false, the setting will not be set if the new value of the
    *          setting is equal to the current value (ie. getSetting()) for the setting
    *          (ie. the value will not be serialized to the user's configuration file).
    *          If true, the setting will be set regardless of the current
    *          value (ie. it will be serialized to the user's configuration file).
    *
    * @return  TRUE if the setting was successfully set, and FALSE otherwise.
    *
    * @see     DynamicObject::setAttribute
    */
   virtual bool setSetting(const std::string &key, const DataVariant &var, bool setIfSame = false) = 0;

   /**
    * Sets the given setting for this session only.  Any value
    * set using this method will only be valid while the application
    * is running.
    *
    * @param key
    *        The name of the setting.  This key
    *        can have '/' in it, in which it will behave like DynamicObject::setAttributeByPath.
    * @param var
    *        The value of the setting. If this is invalid, no setting will 
    *        be added or modified.
    *
    * @return TRUE if the setting was successfully set, and FALSE otherwise.
    */
   virtual bool setSessionSetting(const std::string& key, const DataVariant& var) = 0;

   /**
    * Gets the current value for the given setting.  The value
    * returned will be located in the following order: session setting,
    * per-user setting and default setting.  
    *
    * @see setSessionSetting(), setSetting()
    *
    * @param   key
    *          Name of the setting to be found.  This key
    *          can have '/' in it, in which case it will behave like
    *          DynamicObject::getAttributeByPath.
    *
    * @return  The setting in a DataVariant. If the setting was not found,
    *          an invalid DataVariant will be returned.
    */
   virtual const DataVariant &getSetting(const std::string& key) const = 0;

   /**
    * Queries whether a given setting has a per-user value defined.
    *
    * @param key
    *        Name of the setting to be found.  This key
    *        can have '/' in it, in which case it will behave like
    *        DynamicObject::getAttributeByPath.
    *
    * @return true if this particular setting has a per-user value set and
    *         does not have a per-session value set.
    */
   virtual bool isUserSetting(const std::string& key) const = 0;

   /**
    * Removes a per-user setting.  See getSetting() for
    * more details.
    *
    * @param   key
    *          Name of the setting to be deleted.
    */
   virtual void deleteUserSetting(const std::string& key) = 0;

   /**
    * Removes a per-session setting.  See getSetting() for
    * more details.  
    *
    * @param   key
    *          Name of the setting to be deleted.
    */
   virtual void deleteSessionSetting(const std::string& key) = 0;

   /**
    * Copies the value of the given setting into the provided
    * DynamicObject. This method is intented to be used
    * when creating default files, so a plug-in developer
    * can copy a setting into a DynamicObject which can
    * be written to a defaults file using serializeAsDefaults().
    *
    * @param key
    *        Name of the setting to copy into the DynamicObject.
    * @param pObject
    *        The DynamicObject that the setting should be copied into.
    *        It will be copied in, such that it can be retrieved by
    *        calling pObject->getAttributeByPath(group + "/" + key);
    */
   virtual void copySetting(const std::string& key, DynamicObject* pObject) const = 0;

   /**
    * Saves the given DynamicObject values into a .cfg file that could be put
    * into the AppHome/DefaultSettings directory and be used as defaults
    * by the application.
    *
    * @param pFilename
    *        The full path of the file where the dynamic object should be saved.
    * @param pObject
    *        The settings that should be saved into the file.
    *
    * @return TRUE if the save was successful, otherwise FALSE.
    */
   virtual bool serializeAsDefaults(const Filename* pFilename, const DynamicObject* pObject) const = 0;

   /**
    * Loads the given settings from the file and return them as a 
    * DynamicObject. The load settings are simply returned as a
    * dynamic object, existing setting values are left unchanged.
    *
    * @param pFilename
    *        the full path to the file containing the settings.
    *
    * @return A dynamic object containing the loaded settings or NULL
    *         if the settings could not be loaded.
    */
   virtual DynamicObject* deserialize(const Filename* pFilename) const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~ConfigurationSettings() {}
};

/**
*  Extends capability of the ConfigurationSettings interface.
*
*  This class provides additional capability for the ConfigurationSettings interface class.
*  A pointer to this class can be obtained by performing a dynamic cast on a
*  pointer to ConfigurationSettings or any of its subclasses.
*
*  @warning A pointer to this class can only be used to call methods contained
*           in this extension class and cannot be used to call any methods in
*           ConfigurationSettings or its subclasses.
*/
class ConfigurationSettingsExt1
{
public:
   /**
   *  Get path to user's documents folder.
   *
   *  The path to the current user's documents folder is returned. Under Windows this is usually
   *  C:\\Documents and Settings\\username\\My Documents\\%Opticks, and under Solaris it is 
   *  usually /export/home/username/Opticks .
   *
   *  @return  string
   *           The full path to the user's documents folder.
   */
   virtual std::string getUserDocs() const = 0;

protected:
   /**
   * This will be cleaned up during application close.  Plug-ins do not
   * need to destroy it.
   */
   virtual ~ConfigurationSettingsExt1() {}
};

#endif
