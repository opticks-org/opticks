/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include "SessionItem.h"

#include <map>
#include <string>

class SessionItemId;

/**
 *  Descriptive abstract interface to all plug-ins.
 *
 *  Defines the descriptive abstract interface to all plug-ins.  This
 *  interface is used to query all plug-ins for textual descriptive
 *  information.
 *
 *  @see        SessionItem
 */
class PlugIn : public SessionItem
{
public:
   /**
    *  Returns the plug-in version.
    *
    *  This method returns the plug-in version as a string.
    *  For example, "1.1.1" is a valid version string.
    *
    *  @return  The plug-in version.
    */
   virtual std::string getVersion() const = 0;

   /**
    *  Returns the plug-in production status.
    *
    *  This method returns the plug-in production status as a bool.
    *
    *  @return  The plug-in production status.
    */
   virtual bool isProduction() const = 0;

   /**
    *  Returns the name of the organization that created the plug-in.
    *
    *  This method returns the string name of the organization
    *  responsible for creating the plug-in.  For example,
    *  "Ball Aerospace & Technologies Corp." is a valid creator string.
    *
    *  @return  The organization that created the plug-in.
    */
   virtual std::string getCreator() const = 0;

   /**
    *  Returns the full copyright information for the plug-in.
    *
    *  @return  The copyright information for the plug-in.
    */
   virtual std::string getCopyright() const = 0;

   /**
    *  Returns copyright information for plug-in dependencies.
    *
    *  This information will appear in the About box. If information
    *  for a given dependency name already exists, it will not be replaced.
    *  The copyright message may be formatted with HTML markup.
    *
    *  @return A map containing copyright information for dependencies.
    *          The key should be a short name for the dependency. (for example: libtiff)
    *          The value should be the verbatim copyright message for that plug-in.
    */
   virtual std::map<std::string, std::string> getDependencyCopyright() const = 0;

   /**
    *  Returns a text description for the plug-in.
    *
    *  This method returns the full textual description for
    *  a plug-in.  The description string is of arbitrary length.
    *
    *  @return  The full plug-in description.
    *
    *  @see     getShortDescription()
    */
   virtual std::string getDescription() const = 0;

   /**
    *  Returns a short description for the plug-in.
    *
    *  This method returns a short 50-charater or less 
    *  description about the plug-in.  This short description 
    *  is used for brief descriptive table entries in the GUI.
    *
    *  @return  The short plug-in description.
    *
    *  @see     getDescription()
    */
   virtual std::string getShortDescription() const = 0;

   /**
    *  Returns a unique id for the type of plug-in.
    *
    *  This method returns a unique id for the plug-in class. This string  
    *  should be formatted as a UUID. This value must be the same between
    *  instances of the application.
    *
    *  @return  The UUID
    */
   virtual std::string getDescriptorId() const = 0;

   /**
    *  Returns the plug-in type.
    *
    *  Default plug-in types include the following:
    *  <ul>
    *    <li>PlugInManagerServices::AlgorithmType()</li>
    *    <li>PlugInManagerServices::ExporterType()</li>
    *    <li>PlugInManagerServices::GeoreferenceType()</li>
    *    <li>PlugInManagerServices::ImporterType()</li>
    *    <li>PlugInManagerServices::InterpreterType()</li>
    *    <li>PlugInManagerServices::OptionType()</li>
    *    <li>PlugInManagerServices::PropertiesType()</li>
    *    <li>PlugInManagerServices::RasterPagerType()</li>
    *    <li>PlugInManagerServices::ViewerType()</li>
    *    <li>PlugInManagerServices::WizardType()</li>
    *  </ul>
    *
    *  @return  The plug-in type.
    */
   virtual std::string getType() const = 0;

   /**
    *  Returns the plug-in subtype.
    *
    *  Plug-ins can optionally define a subtype to further distinguish between
    *  multiple kinds of plug-ins with the same type, typically importers and
    *  exporters.  The subtype is used internally to populate file selection
    *  dialogs so that they contain only a subset of plug-ins of a given type.
    *  Default plug-in subtypes include the following:
    *  <ul>
    *    <li>%Layer</li>
    *    <li>Model Element</li>
    *    <li>Product</li>
    *    <li>Raster Element</li>
    *    <li>Shape File</li>
    *    <li>%Signature</li>
    *    <li>%Signature Set</li>
    *  </ul>
    *
    *  @return  The plug-in subtype.  An empty string may be returned,
    *           indicating that the plug-in does not have a specific
    *           subtype.
    *
    *  @see     getType()
    */
   virtual std::string getSubtype() const = 0;

   /**
    *  Queries whether multiple instances of the plug-in can be instantiated
    *  simultaneously.
    *
    *  If multiple instances of a plug-in are allowed, the plug-in should have
    *  no static variables and can have instance-independent input and output
    *  files.
    *
    *  @return  Returns \b true if the multiple instances of the plug-in can be
    *           created and executed simultaneously; otherwise returns
    *           \b false.
    */
   virtual bool areMultipleInstancesAllowed() const = 0;

   /**
    *  @copydoc SettableSessionItem::setId()
    */
   virtual bool setId(const SessionItemId& id) = 0;

protected:
   /**
    * This should be destroyed by calling PlugInManagerServices::destroyPlugIn.
    */
   virtual ~PlugIn() {}

private:
   friend class ModuleDescriptor;
   friend class PlugInDescriptorImp;
   friend class CoreModuleDescriptor;
};

#endif
