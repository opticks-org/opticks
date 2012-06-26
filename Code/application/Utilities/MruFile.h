/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MRUFILE_H
#define MRUFILE_H

#include "DateTimeImp.h"
#include "Serializable.h"

#include <string>
#include <vector>

class ImportDescriptor;

class MruFile : public Serializable
{
public:
   MruFile();
   MruFile(const std::string& name, const std::string& importerName,
      const std::vector<ImportDescriptor*>& descriptors, const DateTimeImp& modificationTime);

   virtual ~MruFile();
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getName() const;
   const std::string& getImporterName() const;
   const std::vector<ImportDescriptor*>& getDescriptors() const;
   const DateTimeImp& getModificationTime() const;

private:
   std::string mName;
   std::string mImporterName;
   std::vector<ImportDescriptor*> mDescriptors;
   DateTimeImp mModificationTime;

   // Not implemented.
   MruFile(const MruFile&);
   MruFile& operator=(const MruFile&);
};

#endif
