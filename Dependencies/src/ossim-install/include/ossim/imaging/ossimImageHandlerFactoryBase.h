//*******************************************************************
//
// License: MIT
// 
// Author:  Garrett Potts
//
//*******************************************************************
// $Id$

#ifndef ossimImageHandlerFactoryBase_HEADER
#define ossimImageHandlerFactoryBase_HEADER 1

#include <ossim/base/ossimString.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/support_data/ImageHandlerState.h>
#include <ossim/base/ossimObjectFactory.h>
#include <ossim/base/ossimIosFwd.h>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <vector>

class ossimFilename;
class ossimKeywordlist;

class OSSIM_DLL ossimImageHandlerFactoryBase : public ossimObjectFactory
{
public:
   template <class T> class UniqueList
   {
   public:
      
      void push_back(const T& value)
      {
         if(std::find(theList.begin(),theList.end(), value) == theList.end())
         {
            theList.push_back(value);
         }
      }

      const std::vector<T>& getList()const
      {
         return theList;
      }
      ossim_uint32 size()const
      {
         return theList.size();
      }
      const T& operator[](ossim_uint32 idx)const
      {
         return theList[idx];
      }
      T& operator[](ossim_uint32 idx)
      {
         return theList[idx];
      }
   protected:
      std::vector<T> theList;
   };
   
   typedef UniqueList<ossimString> UniqueStringList;
   typedef std::vector<ossimRefPtr<ossimImageHandler> > ImageHandlerList;
   
   virtual ossimImageHandler* open(const ossimFilename& fileName,
                                   bool openOverview=true)const = 0;
   virtual ossimImageHandler* open(const ossimKeywordlist& kwl,
                                   const char* prefix=0)const = 0;
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
   virtual ossimRefPtr<ossimImageHandler> open( ossim::istream* str,
                                                std::streamoff restartPosition,
                                                bool youOwnIt ) const;
#endif
   
   /**
    * @brief Open method that takes a stream.
    *
    * This implementation returns an ossimRefPtr with a null pointer.
    * 
    * @param str Open stream to image.
    * @param connectionString
    * @param openOverview If true attempt to open overview file. 
    * @return ossimImageHandler
    */
   virtual ossimRefPtr<ossimImageHandler> open(
      std::shared_ptr<ossim::istream>& str,
      const std::string& connectionString,
      bool openOverview=true ) const;


   virtual ossimRefPtr<ossimImageHandler> open(std::shared_ptr<ossim::ImageHandlerState> state)const;


   /**
    * @brief Open overview that takes a file name.
    *
    * This default implementation returns a null ref pointer.
    * Derived factories that have overview readers should override.
    * 
    * @param file File to open.
    *
    * @return This default implementation returns a null ref pointer.
    */
   virtual ossimRefPtr<ossimImageHandler> openOverview(
      const ossimFilename& file ) const;

   /**
    * @brief Open method that takes a stream.
    *
    * This implementation returns an ossimRefPtr with a null pointer.
    * 
    * @param str Open stream to image.
    * @param connectionString
    * @return ossimImageHandler
    */
   virtual ossimRefPtr<ossimImageHandler> openOverview(
      std::shared_ptr<ossim::istream>& str,
      const ossimString& connectionString ) const;


   virtual void getImageHandlersBySuffix(ImageHandlerList& result,
                                         const ossimString& ext)const;

   virtual void getImageHandlersByMimeType(ImageHandlerList& result,
                                           const ossimString& mimeType)const;

   virtual void getSupportedExtensions(ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const=0;
   
TYPE_DATA
};

#endif
