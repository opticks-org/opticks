/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEMPLATEUTILITIES_H
#define TEMPLATEUTILITIES_H

#include "DesktopItems.h"

#include <string>

class Filename;
class ProductView;

class TemplateUtilities : public DesktopItems
{
public:
   TemplateUtilities();
   ~TemplateUtilities();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   virtual bool executeUtility(ProductView* pView, const std::string& templateFile) = 0;

private:
   std::string mProductName;
   Filename* mpFilename;
};

////////////////////////////////////////////////////////////
class LoadTemplate : public TemplateUtilities
{
public:
   LoadTemplate();
   ~LoadTemplate();

protected:
   virtual bool executeUtility(ProductView* pView, const std::string& templateFile);
};

////////////////////////////////////////////////////////////
class SaveTemplate : public TemplateUtilities
{
public:
   SaveTemplate();
   ~SaveTemplate();

protected:
   virtual bool executeUtility(ProductView* pView, const std::string& templateFile);
};

#endif
