/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTDESCRIPTOR_H
#define IMPORTDESCRIPTOR_H

#include "DataDescriptor.h"

#include <vector>

/**
 *  Specifies parameters for importing a single DataElement.
 *
 *  The import descriptor contains all parameters necessary to import a single
 *  DataElement, including a DataDescriptor and a flag that specifies whether
 *  the data should be included or ignored on import.
 *
 *  Importers should create an import descriptor by calling
 *  ModelServices::createImportDescriptor() from within the
 *  Importer::getImportDescriptors() method.
 *
 *  @see        DataDescriptor
 */
class ImportDescriptor
{
public:
   /**
    *  Sets the data set to import.
    *
    *  This method sets a new underlying data descriptor for import and deletes
    *  the previous data descriptor.  The import descriptor assumes ownership
    *  of the given data descriptor and will delete it when the import
    *  descriptor is destroyed or on the next call to setDataDescriptor().
    *
    *  @param   pDescriptor
    *           The data descriptor to import.  Ownership is transferred to the
    *           import descriptor.
    */
   virtual void setDataDescriptor(DataDescriptor* pDescriptor) = 0;

   /**
    *  Returns the data set for import.
    *
    *  @return  A pointer to the underlying data descriptor to import.
    */
   virtual DataDescriptor* getDataDescriptor() const = 0;

   /**
    *  Sets whether the data element represented by the underlying data
    *  descriptor should be imported.
    *
    *  This method provides a means by which the underlying data descriptor can
    *  be ignored on import.  Importers can call this method to default the
    *  data to load on import or to ignore loading the data on import.  In
    *  interactive mode the application will call this method to set the value
    *  based the user selects in the import dialog.
    *
    *  By default when an ImportDescriptor is created, the import flag is set
    *  to \c true.
    *
    *  @param   bImport
    *           Set this value to \c true to import the given data element or
    *           to \c false to ignore the data element when importing.
    */
   virtual void setImported(bool bImport) = 0;

   /**
    *  Queries whether the data element represented by the underlying data
    *  descriptor should be imported.
    *
    *  By default when an ImportDescriptor is created, this method returns
    *  \c true.
    *
    *  @return  Returns \c true if the data represented by the underlying data
    *           descriptor should be imported, or \c false if the data should
    *           be ignored on import.
    *
    *  @see     setImported()
    */
   virtual bool isImported() const = 0;

   /**
    *  Retrieves imported data sets from given import descriptors.
    *
    *  @param   importDescriptors
    *           The import descriptors over which to search for imported data
    *           sets.
    *
    *  @return  Returns a vector populated with const DataDescriptor pointers
    *           representing the imported data sets contained in the given
    *           import descriptors.
    *
    *  @see     isImported()<br>Importer::validate()
    */
   static std::vector<const DataDescriptor*>
      getImportedDataDescriptors(const std::vector<ImportDescriptor*>& importDescriptors)
   {
      std::vector<const DataDescriptor*> importedDataDescriptors;
      for (std::vector<ImportDescriptor*>::const_iterator iter = importDescriptors.begin();
         iter != importDescriptors.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         if ((pImportDescriptor != NULL) && (pImportDescriptor->isImported() == true))
         {
            DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
            if (pDescriptor != NULL)
            {
               importedDataDescriptors.push_back(pDescriptor);
            }
         }
      }

      return importedDataDescriptors;
   }

protected:
   /**
    *  This object should be destroyed by calling
    *  ModelServices::destroyImportDescriptor().
    */
   virtual ~ImportDescriptor() {}
};

#endif
