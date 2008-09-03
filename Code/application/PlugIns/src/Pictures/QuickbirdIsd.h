/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef QUICKBIRDISD_H__
#define QUICKBIRDISD_H__

#include <string>

class DynamicObject;

class QuickbirdIsd
{
public:
   QuickbirdIsd(const std::string &filename);
   ~QuickbirdIsd();

   /**
    * Copy data from the ISD file to a DynamicObject.
    *
    * This only copies information needed to run RpcGeoreference.
    */
   bool copyToMetadata(DynamicObject *pMetadata) const;

private:
   std::string mFilename;
};

#endif
