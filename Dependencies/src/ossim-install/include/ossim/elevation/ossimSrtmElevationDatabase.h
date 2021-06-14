//---
//
// License: MIT
//
//---
// $Id$

#ifndef ossimSrtmElevationDatabase_HEADER
#define ossimSrtmElevationDatabase_HEADER 1

#include <ossim/elevation/ossimElevationCellDatabase.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/elevation/ossimSrtmHandler.h>
#include <mutex>

class OSSIM_DLL ossimSrtmElevationDatabase : public ossimElevationCellDatabase
{
public:
   typedef std::vector<ossimRefPtr<CellInfo> > DirectMap; // 360x180 cell grid
   
   ossimSrtmElevationDatabase();

   ossimSrtmElevationDatabase(const ossimSrtmElevationDatabase& rhs);

   virtual ~ossimSrtmElevationDatabase();

   virtual ossimObject* dup() const;

   virtual bool open(const ossimString& connectionString);
   bool getAccuracyInfo(ossimElevationAccuracyInfo& info, const ossimGpt& /*gpt*/) const;
   virtual bool pointHasCoverage(const ossimGpt& gpt) const
   {
      ossimFilename filename;
      createFullPath(filename, gpt);
      
      return filename.exists();
   }
   virtual double getHeightAboveMSL(const ossimGpt&);
   virtual double getHeightAboveEllipsoid(const ossimGpt& gpt);
   virtual ossim_uint64 createId(const ossimGpt& pt)const
   {
      ossim_uint64 y = static_cast<ossim_uint64>(ossim::wrap(pt.latd(), -90.0, 90.0)+90.0);
      ossim_uint64 x = static_cast<ossim_uint64>(ossim::wrap(pt.lond(),-180.0,180.0)+180.0);
      // map the extreme edge to the same ID ax the 179 west cell and the same for the 89
      // degree north cell.
      //
      x = x==360?359:x;
      y = y==180?179:y;
      // Srtm databases are 1x1 degree cells and we will use a world 
      // grid for id generation.
      //
      return (y*360+x);
   }
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix = 0);
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix = 0)const;
   
protected:
   bool openSrtmDirectory(const ossimFilename& dir);
   void createRelativePath(ossimFilename& file, const ossimGpt& gpt)const;
   void createFullPath(ossimFilename& file, const ossimGpt& gpt)const
   {
      ossimFilename relativeFile;
      createRelativePath(relativeFile, gpt);
      file = ossimFilename(m_connectionString).dirCat(relativeFile);
   }

   ossimRefPtr<ossimElevCellHandler> createCell(const ossimGpt& gpt);

   TYPE_DATA
};
#endif
