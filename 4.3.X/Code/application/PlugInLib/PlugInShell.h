/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINSHELL_H
#define PLUGINSHELL_H

#include "ObjectResource.h"
#include "PlugIn.h"

#include <list>
#include <string>
#include <vector>

class Progress;
class QAction;
class QIcon;
class SessionItemId;
class SettableSessionItem;

/**
 * Report failure if a condition is not met.
 *
 * There must be variables named pProgress and pStep in order to
 * use this macro, of types Progress* and StepResource, respectively.

 * @param cond
 *        The condition to check again.
 * @param errorMessage
 *        The message, implicitly convertable to a std::string, to report.
 * @param action
 *        The action to take in case of failure.  For example, "return", "continue", "break"
 */
#define FAIL_IF(cond, errorMessage, action) \
   if (cond) \
   { \
      if (pProgress != NULL) \
      { \
         pProgress->updateProgress(errorMessage, 0, ERRORS); \
      } \
      pStep.get()->finalize(Message::Failure, errorMessage); \
      action; \
   }

/**
 *  \ingroup ShellModule
 *  A base class for plug-in shells or plug-in instances.
 *
 *  This class provides a default implementation of the PlugIn
 *  interfaces and serves as an optional base class for specialized
 *  plug-in shell classes and/or custom plug-ins.
 *
 *  @see     PlugIn
 */
class PlugInShell : public PlugIn, public SessionItemExt1
{
public:
   /**
    *  Creates the plug-in.
    *
    *  The constructor initializes the plug-in as follows:
    *  - getVersion() returns the same version as the main application with
    *    which the plug-in was compiled.
    *  - isProduction() returns \b false.
    *  - areMultipleInstancesAllowed() returns \b false.
    *
    *  @see     ConfigurationSettings::getVersion()
    */
   PlugInShell();

   /**
    *  Destroys the plug-in.
    */
   virtual ~PlugInShell();

   // SessionItem
   const std::string& getId() const;

   // SessionItemExt1
   bool isValidSessionSaveItem() const;

   /**
    * @copydoc SessionItem::getIcon()
    *
    * @default This implementation will always return an
    * empty QIcon.  If you would like
    * to return a custom QIcon, you must
    * override this method in your subclass.
    * In order to properly support batch
    * mode, you should only construct
    * the QIcon when this method is
    * called and do not construct
    * the QIcon in your subclasses
    * constructor.
    */
   const QIcon& getIcon() const;
   const std::string& getName() const;
   const std::string& getDisplayName() const;
   const std::string& getDisplayText() const;
   std::list<ContextMenuAction> getContextMenuActions() const;
   bool hasFilenameDisplay() const;
   std::vector<std::string> getPropertiesPages() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);
   /**
    *  @copydoc SettableSessionItem::setId()
    */
   virtual bool setId(const SessionItemId& id);

   // PlugIn
   std::string getVersion() const;
   bool isProduction() const;
   std::string getCreator() const;
   std::string getCopyright() const;
   std::map<std::string, std::string> getDependencyCopyright() const;
   std::string getDescription() const;
   std::string getShortDescription() const;
   std::string getDescriptorId() const;
   std::string getType() const;
   std::string getSubtype() const;
   bool areMultipleInstancesAllowed() const;

protected:
   /**
    *  @copydoc SettableSessionItem::setName()
    */
   void setName(const std::string& name);

   /**
    *  @copydoc SettableSessionItemExt1::setValidSessionSaveItem()
    */
   void setValidSessionSaveItem(bool isValid);

   /**
    *  @copydoc SettableSessionItem::setDisplayName()
    */
   void setDisplayName(const std::string& displayName);

   /**
    *  @copydoc SettableSessionItem::setDisplayText()
    */
   void setDisplayText(const std::string& displayText);

   /**
    *  @copydoc SettableSessionItem::addContextMenuAction()
    */
   void addContextMenuAction(const ContextMenuAction& menuAction);

   /**
    *  @copydoc SettableSessionItem::setContextMenuActions()
    */
   void setContextMenuActions(const std::list<ContextMenuAction>& actions);

   /**
    *  @copydoc SettableSessionItem::setFilenameDisplay()
    */
   void setFilenameDisplay(bool bFilenameDisplay);

   /**
    *  @copydoc SettableSessionItem::addPropertiesPage()
    */
   void addPropertiesPage(const std::string& plugInName);

   /**
    *  @copydoc SettableSessionItem::removePropertiesPage()
    */
   void removePropertiesPage(const std::string& plugInName);

   /**
    *  @copydoc SettableSessionItem::setPropertiesPages()
    */
   void setPropertiesPages(const std::vector<std::string>& plugInNames);

   /**
    *  Sets the plug-in version.
    *
    *  This method sets the plug-in version as a string.
    *  For example, "1.1.1" is a valid version string.
    *
    *  @param   version
    *           The plug-in version.
    */
   void setVersion(const std::string& version);

   /**
    *  Sets the plug-in production status.
    *
    *  This method sets the plug-in production status as a bool.
    *
    *  @param  productionStatus
    *          Is the plug-in a production plug-in?
    */
   void setProductionStatus(bool productionStatus);

   /**
    *  Sets the name of the organization that created the plug-in.
    *
    *  This method sets the string name of the organization
    *  responsible for creating the plug-in.  For example,
    *  "Ball Aerospace & Technologies Corp." is a valid creator string.
    *
    *  @param   creator
    *           The organization that created the plug-in.
    */
   void setCreator(const std::string& creator);

   /**
    *  Sets the full copyright information for the plug-in.
    *
    *  @param   copyright
    *           The copyright information for the plug-in.
    */
   void setCopyright(const std::string& copyright);

   /**
    *  Adds a dependency copyright message.
    *
    *  @param dependencyName
    *         The name identifying the dependency. For example: 'libtiff' or 'qt'.
    *  @param copyright
    *         The verbatim copyright message for the dependency.
    *         The copyright message may be formatted with HTML markup.
    */
   void addDependencyCopyright(const std::string& dependencyName, const std::string& copyright);

   /**
    *  Sets a text description for the plug-in.
    *
    *  This method sets the full textual description for a
    *  plug-in.  The description string can be of arbitrary
    *  length.
    *
    *  @param   description
    *           The full plug-in description.
    *
    *  @see     setShortDescription()
    */
   void setDescription(const std::string& description);

   /**
    *  Sets a short description for the plug-in.
    *
    *  This method sets a short 50-charater or less 
    *  description about the plug-in.  This short description 
    *  is used for brief descriptive table entries in the GUI.
    *
    *  @param   shortDescription
    *           The short plug-in description.
    *
    *  @see     setDescription()
    */
   void setShortDescription(const std::string& shortDescription);

   /**
    *  Sets unique for the plug-in descriptor.
    *
    *  This method sets a unique id for the plug-in class. This string  
    *  should be formatted as a UUID. This value must be the same between
    *  instances of the application.
    *
    *  @param   id
    *           The unique id for the descriptor.
    */
   void setDescriptorId(const std::string& id);

   /**
    *  Sets the plug-in type.
    *
    *  Default plug-in types include the following:
    *  <ul>
    *    <li>Algorithm</li>
    *    <li>Custom Options</li>
    *    <li>Exporter</li>
    *    <li>Georeference</li>
    *    <li>Importer</li>
    *    <li>Interpreter</li>
    *    <li>RasterPager</li>
    *    <li>Viewer</li>
    *    <li>Wizard</li>
    *  </ul>
    *
    *  @param   type
    *           The plug-in type.
    */
   void setType(const std::string& type);

   /**
    *  Sets the plug-in subtype.
    *
    *  Plug-ins can optionally define a subtype to further distinguish between
    *  multiple kinds of plug-ins with the same type, typically importers and
    *  exporters.  The subtype is used internally to populate file selection
    *  dialogs so that they contain only a subset of plug-ins of a given type.
    *  Default plug-in subtypes include the following:
    *  <ul>
    *    <li>Product</li>
    *    <li>%RasterElement</li>
    *    <li>%Signature</li>
    *    <li>%SignatureSet</li>
    *  </ul>
    *
    *  @param   subtype
    *           The plug-in subtype.
    */
   void setSubtype(const std::string& subtype);

   /**
    *  Sets whether multiple instances of the plug-in can be instantiated
    *  simultaneously.
    *
    *  @param   bMultipleInstances
    *           Set this value to true if the multiple instances of the
    *           plug-in can be created and executed simultaneously,
    *           otherwise false.
    */
   void allowMultipleInstances(bool bMultipleInstances);

private:
   FactoryResource<SettableSessionItem> mpSessionItem;
   std::string mVersion;
   bool mProductionStatus;
   std::string mCreator;
   std::string mCopyright;
   std::map<std::string, std::string> mDependencyCopyright;
   std::string mDescription;
   std::string mShortDescription;
   std::string mDescriptorId;
   std::string mType;
   std::string mSubtype;
   bool mAllowMultipleInstances;
};

#endif