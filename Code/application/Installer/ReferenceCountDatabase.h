/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef REFERENCECOUNTDATABASE_H__
#define REFERENCECOUNTDATABASE_H__

#include <QtCore/QHash>
#include <QtCore/QString>

class ReferenceCountDatabase
{
public:
   /**
    * Get the reference count for the specified file.
    *
    * This assumes the file exists (i.e. has a reference count
    * of at least 1)
    *
    * @param filename
    *        Obtain the reference count for this file.
    *        The filename will be normalized and converted
    *        to a path relative to the application directory.
    *
    * @return The reference count for the file. If there is no
    *         stored reference count, a value of 1 is returned.
    */
   QHash<QString, int>::iterator getReferenceCount(const std::string& filename);
   
   void saveDb() const;

protected:
   friend class InstallerServicesImp;
   /**
    * Create a new reference count database accessor.
    */
   ReferenceCountDatabase();

   /**
    * Cleanup
    */
   virtual ~ReferenceCountDatabase();

private:
   void loadDb();

   QHash<QString, int> mDb;

   QString normalizePath(const std::string& filename);
   bool exists(const QString& normalizedFilename);
};

#endif
