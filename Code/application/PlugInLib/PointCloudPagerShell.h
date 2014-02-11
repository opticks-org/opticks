/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDPAGERSHELL_H
#define POINTCLOUDPAGERSHELL_H

#include "ExecutableShell.h"
#include "PointCloudPager.h"

/**
 *  \ingroup ShellModule
 */
class PointCloudPagerShell : public ExecutableShell, public PointCloudPager
{
public:
   /**
    *  Creates a point cloud pager plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::PointCloudPagerType(), sets the plug-in
    *  to allow multiple instances, and to not be destroyed after execution.
    *
    *  @see     getType(), areMultipleInstancesAllowed(),
    *           isDestroyedAfterExecute()
    */
   PointCloudPagerShell();

   /**
    *  Destroys the point cloud pager plug-in.
    */
   ~PointCloudPagerShell();

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default The default implementation does not set any args in the arg
    *           list and returns \b true.
    */
   bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  The default pager session serialization method.
    *
    *  Since pagers don't know enough to recreate and restore themselves on
    *  session load, they must be created by the PointCloudElement or another plug-
    *  in. The default method simply returns true, causing the session load to
    *  not recreate the pager.
    *
    *  @param serializer
    *           The object to use to save the item as part of the current 
    *           session.
    *
    *  @return  \c true if the item was successfully saved and \c false otherwise.
    */
   bool serialize(SessionItemSerializer& serializer) const;
};

#endif
