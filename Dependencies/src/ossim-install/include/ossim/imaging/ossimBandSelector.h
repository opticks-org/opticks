//*******************************************************************
//
// License: MIT
// 
// Author:  Garrett Potts
//
// Description:  Contains class declaration for ossimBandSelector.
// 
//*******************************************************************
//  $Id$
#ifndef ossimBandSelector_HEADER
#define ossimBandSelector_HEADER 1

#include <ossim/imaging/ossimImageSourceFilter.h>
#include <vector>
 
class ossimImageData;
class ossimImageHandler;

class OSSIM_DLL ossimBandSelector : public ossimImageSourceFilter
{
   
public:
   enum ossimBandSelectorWithinRangeFlagState
   {
      ossimBandSelectorWithinRangeFlagState_NOT_SET      = 0,
      ossimBandSelectorWithinRangeFlagState_IN_RANGE     = 1,
      ossimBandSelectorWithinRangeFlagState_OUT_OF_RANGE = 2,
   };

   ossimBandSelector();
   virtual ~ossimBandSelector();
   
   virtual ossimString getLongName()  const;
   virtual ossimString getShortName() const;

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel=0);

   /**
    * Initializes bandList to the zero based order of output bands.
    */
   virtual void getOutputBandList(std::vector<ossim_uint32>& bandList) const;


   /**
    * @brief Will set to three bands (rgb) out.
    *
    * This will attempt to auto detect the zero based rgb band order from the
    * input image handler.  If the result is < 3 bands we will
    * not increase the band output by adding more bands.  For example,  
    * if the input is grey or thermal then it will just output one band for
    * a default allocation.  Note,  this can be overriden by explicitly setting
    * the output band list.  
    * 
    * If the image handler implemented getRgbBandList(...) it will use that:
    *    bands = 2,1,0 ( or whatever handler thinks rgb is )
    * else if 3 band or greater and can't determine rgb:
    *    bands = 0,1,2
    * else (less than three bands)
    *    bands = 0
    */
   virtual void setThreeBandRgb();
    
   /**
    * Sets the output band list.
    *
    * @param outputBandList List of zero based bands.
    * @param disablePassThru If true, forces the band selection to be performed by this filter.
    *        Otherwise, the image handler to the left is directed to select bands if it has that
    *        capability (default behavior).
    *
    * @note The input connection should be established prior to calling this.
    *
    * @note Bands can be duplicated.  So if you have a single band input you
    * can do a band list like "1, 1, 1".   Any band within the list must be
    * available from the input.  So if you have a 3 band input bands must be
    * constrained to zero based bands "0, 1, or 2".
    *
    * @note Calling this method does not enable filter so callers should do:
    * theBandSelector->enableSource();
    * theBandSelector->setOutputBandList(bandList);
    */
   virtual void setOutputBandList(const std::vector<ossim_uint32>& outputBandList,
                                  bool disablePassThru=false);
   
   /**
    * Returns the number of bands in a tile returned from this TileSource.
    */
   virtual ossim_uint32 getNumberOfOutputBands() const;

   /**
    * @brief Returns the number of input bands.
    *
    * Overrides ossimImageSourceFilter::getNumberOfInputBands to check for a
    * single image chain band selectable image handler on the input.  If one
    * is present it returns it's number of input bands instead of the number
    * of the input connection's output bands(our input).  This is needed so
    * callers, e.g. band selector dialog box can query the number of bands
    * available.
    *
    * @return Number of bands available.
    */
   virtual ossim_uint32 getNumberOfInputBands()const;   

   virtual void initialize();
   
   virtual double getMinPixelValue(ossim_uint32 band=0)const;
   virtual double getMaxPixelValue(ossim_uint32 band=0)const;
   virtual double getNullPixelValue(ossim_uint32 band=0)const;

   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=NULL)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=NULL);
   /**
    *   Override base class so that a disableSource event does not
    *   reinitialize the object and enable itself.
    */
   //virtual void propertyEvent(ossimPropertyEvent& event);
   //virtual void refreshEvent(ossimRefreshEvent& event);   

   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;						  
   virtual bool isSourceEnabled()const;

protected:

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();

   /**
    * Will check all combinations of the band list to see if its 
    */
   void checkPassThrough();

   /**
    * @return true if all bands in theOutputBandList are less than the
    * number of input's bands.
    */
   bool outputBandsWithinInputRange() const;

   /**
    * @brief Finds and returns band selectable image handler.
    *
    * To get a valid pointer there must be one and only one image handler on
    * the input connection and it must be a band selector.  In other words,
    * must be a single image chain input and
    * ossimImageHandler::isBandSelector() must return true.
    *
    * @return Pointer to image handler wrapped in an ossimRefPtr on success;
    * null, on error.
    */
   ossimRefPtr<ossimImageHandler> getBandSelectableImageHandler() const;

   /**
    * @brief Attempts to derive rgb bands from image handler to the left of the
    * band selector.
    *
    * Must be one and only one image handler on the input connection.
    * In other words, must be a single image chain input.
    *
    * @return true on success, false on error.
    */
   bool getRgbBandList(std::vector<ossim_uint32>& bandList) const;
   
   ossimRefPtr<ossimImageData>           m_tile;
   std::vector<ossim_uint32>             m_outputBandList;
   ossimBandSelectorWithinRangeFlagState m_withinRangeFlag;
   bool                                  m_passThroughFlag;
   bool                                  m_delayLoadRgbFlag;
   bool                                  m_inputIsSelectable;

TYPE_DATA
};

#endif /* #ifndef ossimBandSelector_HEADER */
