/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFPROPERTIES_H__
#define NITFPROPERTIES_H__

#include "Properties.h"
class QWidget;

namespace Nitf
{
   /**
    * A plug-in interface used to specify a GUI for displaying and (optionally) editing TRE and DES data.
    *
    * Data can be viewed and edited using the normal metadata editor but some data benefit
    * from a more taylored GUI. Implementers of this interface will not need to write a second
    * plug-in to attach the GUI to a specific DataElement. Any DataElement with an associated TRE or DES
    * will be automatically displayed in that element's properties dialog.
    */
   class Properties : public ::Properties
   {
   public:
      /**
       * The type that should be returned from PlugIn::getSubtype()
       * for properties plug-ins. This subtype is needed to automatically
       * attach this property widget to a DataElement.
       *
       * @return Returns the type used for properties plug-ins.
       */
      static std::string SubType()
      {
         return "NITF";
      }

      /**
       * The name of the TRE or DES this plug-in displays.
       *
       * @return The name of the TRE or DES. This must exactly match the TRE or DES name
       *         as it appears in the DataElement metadata.
       */
      virtual std::string getTypeName() const = 0;

      /**
       * Determine if this plug-in can display and/or edit the specified metadata.
       *
       * @param metadata
       *        The element metadata sub-tree for the named TRE or DES. This will be rooted at
       *        the metadata TRE or DES node with the same name as SubType().
       * @return \c True if the metadata can be displayed and/or edited, otherwise \c false.
       */
      virtual bool canDisplayMetadata(const DynamicObject& metadata) = 0;

   protected:
      /**
       *  Since the Properties interface is usually used in conjunction with the
       *  PlugIn interface, this should be destroyed by casting to the PlugIn
       *  interface and calling PlugInManagerServices::destroyPlugIn().
       */
      virtual ~Properties() {}
   };
}

#endif
