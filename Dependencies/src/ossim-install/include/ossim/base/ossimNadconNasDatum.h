#ifndef ossimNadconNasDatum_HEADER
#define ossimNadconNasDatum_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimNadconGridDatum.h>
#include <ossim/base/ossimNadconGridFile.h>

class OSSIM_DLL ossimNadconNasDatum : public ossimNadconGridDatum
{
public:
   ossimNadconNasDatum(const ossimFilename& nadconDirectory);

   // Argument holds the source point and datum.  Returns another
   // point with this datum.
   //
   virtual ossimGpt shift(const ossimGpt    &aPt)const;
   TYPE_DATA;
};

#endif
