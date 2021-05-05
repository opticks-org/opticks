//---
//
// License: MIT
// 
// Author:  David Burken
//
// Description: Factory for info objects.
// 
//---
// $Id$

#ifndef ossimInfoFactory_HEADER
#define ossimInfoFactory_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimInfoFactoryInterface.h>

class ossimFilename;
class ossimInfoBase;

/**
 * @brief Info factory.
 */
class OSSIM_DLL ossimInfoFactory : public ossimInfoFactoryInterface
{
public:

   /** virtual destructor */
   virtual ~ossimInfoFactory();

   static ossimInfoFactory* instance();

   /**
    * @brief create method.
    *
    * @param file Some file you want info for.
    *
    * @return ossimInfoBase* on success 0 on failure.  Caller is responsible
    * for memory.
    */
   // virtual ossimInfoBase* create(const ossimFilename& file) const;
   virtual std::shared_ptr<ossimInfoBase> create(const ossimFilename& file) const;
   
   virtual std::shared_ptr<ossimInfoBase> create(std::shared_ptr<ossim::istream>& str,
                                                 const std::string& connectionString)const;
   
private:
   
   /** hidden from use default constructor */
   ossimInfoFactory();

   /** hidden from use copy constructor */
   ossimInfoFactory(const ossimInfoFactory& obj);

   /** hidden from use operator = */
   const ossimInfoFactory& operator=(const ossimInfoFactory& rhs);

   /** The single instance of this class. */
   static ossimInfoFactory* theInstance;
};

#endif /* End of "#ifndef ossimInfoFactory_HEADER" */
