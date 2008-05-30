/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef GETFILENAME_H
#define GETFILENAME_H

#include <QtCore/QString>

#include "DesktopItems.h"

////////////////////////////////////////////////////////////
class GetFilename : public DesktopItems
{
public:
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   GetFilename();
   ~GetFilename();

   bool extractInputArgs(PlugInArgList* pInArgList);
   virtual QString getFilenameFromUser();

   QString mCaption;
   QString mInitialDir;
   QString mFilters;
};

////////////////////////////////////////////////////////////
class GetExistingFilename : public GetFilename
{
public:
   GetExistingFilename();
   ~GetExistingFilename();

protected:
   QString getFilenameFromUser();
};

////////////////////////////////////////////////////////////
class GetNewFilename : public GetFilename
{
public:
   GetNewFilename();
   ~GetNewFilename();

protected:
   QString getFilenameFromUser();
};

////////////////////////////////////////////////////////////
class GetExistingFilenames : public GetFilename
{
public:
   GetExistingFilenames();
   ~GetExistingFilenames();

   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif   // GETFILENAME_H

 
