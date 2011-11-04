/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARCSDK_H
#define ARCSDK_H

#if defined(ESRI_WINDOWS)

#include "atlbase.h"

#include <CatIDs\ArcCATIDs.h>

#pragma warning(push)
#pragma warning(disable : 4146)
#pragma warning(disable : 4192)
#pragma warning(disable : 4336)

#import <esriSystem.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")
#import <esriSystemUI.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")
#import <esriGeometry.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")
#import <esriGeodatabase.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")
#import <esriDataSourcesGDB.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")
#import <esriDisplay.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")
#import <esriGeoDatabase.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")
#import <esriDataSourcesFile.olb> raw_interfaces_only raw_native_types no_namespace named_guids exclude("OLE_COLOR", "OLE_HANDLE")

#pragma warning(pop)

#endif

#endif