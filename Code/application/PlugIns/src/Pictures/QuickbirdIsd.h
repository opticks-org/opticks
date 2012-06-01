/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef QUICKBIRDISD_H
#define QUICKBIRDISD_H

#include <QtCore/QString>

class DynamicObject;

class QuickbirdIsd
{
public:
   QuickbirdIsd(DynamicObject* pMetadata);
   ~QuickbirdIsd();

   bool loadIsdMetadata(const QString& isdFilename);
   QString getIsdFilename() const;
   void removeIsdMetadata();

private:
   DynamicObject* mpMetadata;
};

#endif
