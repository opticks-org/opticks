/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODULEDESCRIPTOR_H
#define MODULEDESCRIPTOR_H 

#include "DateTimeImp.h"
#include "DynamicModule.h"
#include "SessionItem.h"
#include "SessionItemImp.h"

#include <map>
#include <string>
#include <vector>

class DynamicObject;
struct OpticksModuleDescriptor;
class PlugIn;
class PlugInDescriptorImp;
class QDataStream;

class ModuleDescriptor : public SessionItem, public SessionItemImp
{
public:
   static ModuleDescriptor* getModule(const std::string& filename, std::map<std::string, std::string>& plugInIds);

   virtual ~ModuleDescriptor();

   virtual bool load();
   virtual void unload();
   virtual bool initializePlugInInformation(std::map<std::string, std::string>& plugInIds);
   virtual PlugIn* createInterface(unsigned int plugInNumber);
   virtual PlugIn* createInterface(PlugInDescriptorImp* pDescriptor);

   virtual const bool isValidatedModule() const;

   virtual const std::string getVersion() const
   {
      return mVersion;
   }

   virtual const std::string getDescription() const
   {
      return mDescription;
   }

   virtual const std::string getFileName() const
   {
      return mFileName;
   }

   virtual const double getFileSize() const
   {
      return mFileSize;
   }

   virtual const DateTime* getFileDate() const
   {
      return &mFileDate;
   }

   virtual const unsigned int getNumPlugIns() const
   {
      return mPlugInTotal;
   }

   virtual std::vector<PlugInDescriptorImp*> getPlugInSet() const
   {
      return mPlugins;
   }

   virtual bool isLoaded() const
   {
      if (mpModule == NULL)
      {
         return false;
      }

      return mpModule->isLoaded();
   }

   virtual int getModuleVersion() const
   {
      return mModuleVersion;
   }

    /**
    *  Return a flag indicating if the descriptor should be stored in the
    *  plug-in list cache to speed up future startup of the application.
    *
    *  @return  \c true if the module should be cached or \c false otherwise.
    */
   bool canCache() const
   {
      return mCanCache;
   }

   static ModuleDescriptor* fromSettings(const DynamicObject& settings);
   bool updateSettings(DynamicObject& settings) const;

   SESSIONITEMACCESSOR_METHODS(SessionItemImp)

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   ModuleDescriptor(const std::string& id);

   bool populateFromSettings(QDataStream& reader);

   std::string mVersion;
   std::string mDescription;
   unsigned int mPlugInTotal;
   std::string mValidationKey;
   bool mCanCache;

   std::string mInstantiateSymbol;
   int mModuleVersion;

   std::string mFileName;
   double mFileSize;
   DateTimeImp mFileDate;

   std::vector<PlugInDescriptorImp*> mPlugins;
   DynamicModule* mpModule;
};

#define READ_FROM_STREAM(var) reader >> var; if (reader.status() != QDataStream::Ok) { return false; }
#define READ_STR_FROM_STREAM(var) QString qStr##var; READ_FROM_STREAM(qStr##var) var = qStr##var.toStdString();

#endif
