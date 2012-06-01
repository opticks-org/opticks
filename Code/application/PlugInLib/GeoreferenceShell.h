/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOREFERENCESHELL_H
#define GEOREFERENCESHELL_H

#include "ExecutableShell.h"
#include "Georeference.h"
#include "LocationType.h"

/**
 *  \ingroup ShellModule
 */
class GeoreferenceShell : public ExecutableShell, public Georeference
{
public:
   /**
    *  Creates a georeference plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::GeoreferenceType() and sets the
    *  plug-in to allow multiple instances.
    *
    *  @see     getType(), areMultipleInstancesAllowed()
    */
   GeoreferenceShell();

   /**
    *  Destroys the georeference plug-in.
    */
   virtual ~GeoreferenceShell();

   /**
    *  @copydoc Executable::setInteractive()
    *
    *  @default The default implementation sets the plug-in to execute in
    *           interactive mode, but returns \c false to indicate that
    *           interactive mode is not supported.
    */
   bool setInteractive();

   /**
    *  @copydoc Executable::getInputSpecification()
    *
    *  @default The default implementation adds the following args:
    *  <table><tr><th>Name</th>           <th>Type</th></tr>
    *         <tr><td>Raster Element</td> <td>RasterElement</td></tr>
    *         <tr><td>Progress</td>       <td>Progress</td></tr></table>
    */
   bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default The default implementation does not set any args in the arg
    *           list and returns \b true.
    */
   bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Georeference::pixelToGeoQuick()
    *
    *  @default The default implementation calls pixelToGeo().
    */
   LocationType pixelToGeoQuick(LocationType pixel, bool* pAccurate = NULL) const;

   /**
    *  @copydoc Georeference::geoToPixelQuick()
    *
    *  @default The default implementation calls geoToPixel().
    */
   LocationType geoToPixelQuick(LocationType geo, bool* pAccurate = NULL) const;

   /**
    *  @copydoc Georeference::getWidget()
    *
    *  @default The default implementation does nothing and returns \c NULL.
    */
   QWidget* getWidget(RasterDataDescriptor* pDescriptor);

   /**
    *  @copydoc Georeference::validate()
    *
    *  @default The default implementation of this method performs the following
    *           tests:
    *           - Check for a valid (i.e. non-\c NULL) RasterDataDescriptor.
    *           - Check for a valid (i.e. non-\c NULL) GeoreferenceDescriptor.
    *           - If creating a results layer, check for an invalid (i.e. empty)
    *             layer name.
    */
   bool validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const;
};

#endif
