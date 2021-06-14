//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
// Description:  Contains class definition for the class
//               ImageHandlerRegistry.
//
//*******************************************************************
//  $Id: ossimImageHandlerRegistry.h 22636 2014-02-23 17:55:50Z dburken $

#ifndef ossimImageHandlerRegistry_HEADER
#define ossimImageHandlerRegistry_HEADER 1

#include <ossim/base/ossimObjectFactory.h>
#include <ossim/base/ossimRtti.h>
#include <ossim/imaging/ossimImageHandlerFactoryBase.h>
#include <ossim/base/ossimFactoryListInterface.h>
#include <iosfwd>
#include <vector>
#include <memory>
#include <ossim/base/ItemCache.h>
#include <ossim/support_data/ImageHandlerState.h>

class ossimImageHandler;
class ossimFilename;
class ossimKeywordlist;

/**
* ossimImageHandlerRegistry supports the new state cache. During initialization the properties are
* read from the global preferences.  The keywords are:
*
* ossim.imaging.handler.registry.state_cache.enabled: true or false
* ossim.imaging.handler.registry.state_cache.min_size: min number of items
* ossim.imaging.handler.registry.state_cache.max_size: max number of items
*
* On open if the state cache is enabled it will determine if a state exists when a file is passed 
* in to be open and if a state exists it will try to open the handler based on the state.
*/
class OSSIMDLLEXPORT ossimImageHandlerRegistry : public ossimObjectFactory,
                                                public ossimFactoryListInterface<ossimImageHandlerFactoryBase, ossimImageHandler>
{
public:
   virtual ~ossimImageHandlerRegistry();
   
   static ossimImageHandlerRegistry* instance();


   std::shared_ptr<ossim::ImageHandlerState> getState(const ossimString& connectionString, ossim_uint32 entry)const;
   std::shared_ptr<ossim::ImageHandlerState> getState(const ossimString& id)const;

   ossimRefPtr<ossimImageHandler> openConnection(
      const ossimString& connectionString, bool openOverview=true  )const;
   
   /**
    * @brief open that takes a filename.
    * @param fileName File to open.
    * @param trySuffixFirst If true calls code to try to open by suffix first,
    * then goes through the list of available handlers. default=true.
    * @param openOverview If true image handler will attempt to open overview.
    * default = true
    * @return Pointer to image handler or null if cannot open.
    */
   virtual ossimImageHandler* open(const ossimFilename& fileName,
                                   bool trySuffixFirst=true,
                                   bool openOverview=true)const;
   
   /**
    *  Given a keyword list return a pointer to an ImageHandler.  Returns
    *  null if a valid handler cannot be found.
    */
   virtual ossimImageHandler* open(const ossimKeywordlist& kwl,
                                   const char* prefix=0)const;
   
   virtual ossimRefPtr<ossimImageHandler> open(std::shared_ptr<ossim::ImageHandlerState> state)const;
   /**
    * @brief Open method that takes a stream.
    * @param str Open stream to image.
    * @param connectionString
    * @param openOverview If true attempt to open overview file. 
    * @return ossimImageHandler
    */
   virtual ossimRefPtr<ossimImageHandler> open(
      std::shared_ptr<ossim::istream>& str,
      const std::string& connectionString,
      bool openOverview ) const;
   
#if 0   
   /**
    *  @brief Open method.
    *
    *  This open takes a stream, position and a flag.
    *
    *  @param str Open stream to image.
    *
    *  @param restartPosition Typically 0, this is the stream offset to the
    *  front of the image.
    *
    *  @param youOwnIt If true the opener takes ownership of the stream
    *  pointer and will destroy on close.
    *  
    *  @return This implementation returns an ossimRefPtr with a null pointer.
    */
   virtual ossimRefPtr<ossimImageHandler> open(
      ossim::istream* str,
      std::streamoff restartPosition,
      bool youOwnIt ) const;   
#endif
   
   /**
    * @brief Open overview that takes a file name.
    *
    * This will only check readers that can be overview handlers.
    * 
    * @param file File to open.
    * 
    * @return ossimRefPtr to image handler on success or null on failure.
    */
   virtual ossimRefPtr<ossimImageHandler> openOverview(
      const ossimFilename& file ) const;

   /*!
    * Creates an object given a type name.
    */
   virtual ossimObject* createObject(const ossimString& typeName) const;
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual ossimObject* createObject(const ossimKeywordlist& kwl,
                                     const char* prefix=0)const;

   /**
    * openBySuffix will call the mthod getImageHandlersBySuffix and go through
    * each handler to try and open the file.  This should be a faster open
    * for we do not have to do a magic number compare on all prior files and
    * keep opening and closing files.
    * @param openOverview If true image handler will attempt to open overview.
    * default = true
    */
   virtual ossimRefPtr<ossimImageHandler> openBySuffix(const ossimFilename& file,
                                                       bool openOverview=true)const; 
   
   /**
    *
    * Will add to the result list any handler that supports the passed in extensions
    *
    */
   virtual void getImageHandlersBySuffix(ossimImageHandlerFactoryBase::ImageHandlerList& result,
                                         const ossimString& ext)const;
   /**
    *
    * Will add to the result list and handler that supports the passed in mime type
    *
    */
   virtual void getImageHandlersByMimeType(ossimImageHandlerFactoryBase::ImageHandlerList& result,
                                           const ossimString& mimeType)const;
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList( std::vector<ossimString>& typeList ) const;

   virtual void getSupportedExtensions(
      ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const;

   /**
    * @brief Prints list of readers and properties.
    * @param  out Stream to print to.
    * @return std::ostream&
    */
   std::ostream& printReaderProps(std::ostream& out) const;
   
protected:
   ossimImageHandlerRegistry();
   ossimImageHandlerRegistry(const ossimImageHandlerRegistry& rhs);
   const ossimImageHandlerRegistry&
      operator=(const ossimImageHandlerRegistry& rhs);

   void initializeStateCache()const;

   void addToStateCache(ossimImageHandler* handler)const;

   mutable std::shared_ptr<ossim::ItemCache<ossim::ImageHandlerState> > m_stateCache;

   //static ossimImageHandlerRegistry*            theInstance;
   
TYPE_DATA
};

extern "C"
{
   OSSIM_DLL  void* ossimImageHandlerRegistryGetInstance();
}

#endif /* #ifndef ossimImageHandlerRegistry_HEADER */
