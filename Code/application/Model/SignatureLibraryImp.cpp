/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DynamicObject.h"
#include "ImportDescriptor.h"
#include "Importer.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterDataDescriptorImp.h"
#include "RasterUtilities.h"
#include "Resampler.h"
#include "SignatureLibrary.h"
#include "SignatureLibraryImp.h"
#include "SpecialMetadata.h"
#include "switchOnEncoding.h"

#include <algorithm>

using namespace std;

SignatureLibraryImp::SignatureLibraryImp(const DataDescriptorImp& descriptor, const string& id) : 
   SignatureSetImp(descriptor, id),
   mNeedToResample(false)
{
   mpOdre.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &SignatureLibraryImp::ordinateDeleted));
   mpOdre.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &SignatureLibraryImp::ordinateModified));
}

SignatureLibraryImp::~SignatureLibraryImp()
{
   clear();
}

const vector<double>& SignatureLibraryImp::getAbscissa() const
{
   if (mAbscissa.empty())
   {
      return mOriginalAbscissa;
   }
   return mAbscissa;
}

const vector<double>& SignatureLibraryImp::getOriginalAbscissa() const
{
   return mOriginalAbscissa;
}

const RasterElement *SignatureLibraryImp::getOriginalOrdinateData() const
{
   return mpOdre.get();
}

RasterElement *SignatureLibraryImp::getOriginalOrdinateData()
{
   return mpOdre.get();
}

const double *SignatureLibraryImp::getOrdinateData() const
{
   return getOrdinateData(0);
}

namespace
{
   template<typename T>
   void getOriginalAsDouble(T *pSource, unsigned int count, vector<double> &dest)
   {
      copy(pSource, &pSource[count], dest.begin());
   }
}

const double *SignatureLibraryImp::getOrdinateData(unsigned int index) const
{
   const_cast<SignatureLibraryImp*>(this)->resample(mAbscissa);

   if (index < mSignatures.size() && (!mAbscissa.empty() || mpOdre.get() != NULL))
   {
      if (mAbscissa.empty())
      {
         const RasterDataDescriptor* pDesc =
            dynamic_cast<const RasterDataDescriptor*>(mpOdre.get()->getDataDescriptor());
         if (pDesc != NULL)
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setInterleaveFormat(BIP);
            pRequest->setBands(pDesc->getActiveBand(0),
               pDesc->getActiveBand(pDesc->getBandCount() - 1), pDesc->getBandCount());
            pRequest->setRows(pDesc->getActiveRow(index), pDesc->getActiveRow(index), 1);
            DataAccessor da = mpOdre->getDataAccessor(pRequest.release());
            if (da.isValid())
            {
               static vector<double> sOriginalOrdinateData;
               sOriginalOrdinateData.resize(mOriginalAbscissa.size());
               switchOnEncoding(pDesc->getDataType(), getOriginalAsDouble, da->getRow(), mOriginalAbscissa.size(),
                  sOriginalOrdinateData);
               return &sOriginalOrdinateData[0];
            }
         }

         return NULL;
      }

      return &mResampledData[index * getAbscissa().size()];
   }

   return NULL;
}

set<string> SignatureLibraryImp::getSignatureNames() const
{
   set<string> names;
   for (map<string, Signature*>::const_iterator pItem = mSignatureNames.begin();
      pItem != mSignatureNames.end(); ++pItem)
   {
      names.insert(pItem->first);
   }
   return names;
}

string SignatureLibraryImp::getSignatureName(unsigned int index) const
{
   if (index < mSignatures.size())
   {
      return mSignatures[index]->getName();
   }
   else
   {
      return string();
   }
}

const Signature *SignatureLibraryImp::getSignature(unsigned int index) const
{
   const_cast<SignatureLibraryImp*>(this)->resample(mAbscissa);

   if (index < mSignatures.size())
   {
      return mSignatures[index];
   }
   else
   {
      return NULL;
   }
}

const Signature* SignatureLibraryImp::getSignature(const string& name) const
{
   const_cast<SignatureLibraryImp*>(this)->resample(mAbscissa);

   map<string, Signature*>::const_iterator pItem = mSignatureNames.find(name);
   if (pItem != mSignatureNames.end())
   {
      return pItem->second;
   }
   else
   {
      return NULL;
   }
}

bool SignatureLibraryImp::resample(const vector<double> &abscissa)
{
   if (mpOdre.get() == NULL)
   {
      return false;
   }

   if (mResampledData.empty() == false && mNeedToResample == false && abscissa == mAbscissa)
   {
      return true;
   }

   PlugInResource resampler("Resampler");
   Resampler* pResampler = dynamic_cast<Resampler*>(resampler.get());
   if (pResampler == NULL)
   {
      return false;
   }

   desample();

   if (abscissa.empty())
   {
      return true;
   }

   RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(mpOdre->getDataDescriptor());
   VERIFY(pDesc != NULL);

   unsigned int numSigs = pDesc->getRowCount();
   if (numSigs == 0)
   {
      return true;
   }

   mResampledData.resize(numSigs * abscissa.size());

   DataAccessor da = mpOdre->getDataAccessor();
   if (da.isValid() == false)
   {
      desample();
      return false;
   }

   vector<double> originalOrdinateData(mOriginalAbscissa.size());
   vector<double> toData;
   vector<double> toFwhm;
   vector<int> toBands;
   string errorMessage;
   for (unsigned int i = 0; i < mSignatures.size(); ++i)
   {
      switchOnEncoding(pDesc->getDataType(), getOriginalAsDouble, da->getRow(), mOriginalAbscissa.size(),
         originalOrdinateData);
      toData.clear();
      toData.reserve(mAbscissa.size());
      toBands.clear();
      toBands.reserve(mAbscissa.size());
      bool success = pResampler->execute(originalOrdinateData, toData, 
         mOriginalAbscissa, abscissa, toFwhm, toBands, errorMessage);
      if (!success || toData.size() != abscissa.size())
      {
         desample();
         return false;
      }
      std::copy(toData.begin(), toData.end(), &mResampledData[i*abscissa.size()]);
      da->nextRow();
   }

   mAbscissa = abscissa;

   mNeedToResample = false;

   notify(SIGNAL_NAME(Subject, Modified));

   return true;
}

void SignatureLibraryImp::desample()
{
   bool needToSignal = (!mAbscissa.empty()) || (!mResampledData.empty());
   mAbscissa.clear();
   mResampledData.clear();
   mNeedToResample = false;
   if (needToSignal)
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool SignatureLibraryImp::import(const string &filename, const string &importerName, Progress* pProgress)
{
   ImporterResource importer(importerName, filename, pProgress);
   vector<ImportDescriptor*> descs = importer->getImportDescriptors();

   RasterDataDescriptor* pCubeDescriptor = NULL;
   if (descs.size() == 1 && descs.front() != NULL)
   {
      pCubeDescriptor = dynamic_cast<RasterDataDescriptor*>(descs.front()->getDataDescriptor());
   }

   if (pCubeDescriptor != NULL)
   {
      pCubeDescriptor->setProcessingLocation(ON_DISK);
      bool cubeSuccess = importer->execute();
      if (cubeSuccess)
      {
         vector<DataElement*> importedElements = importer->getImportedElements();
         if (!importedElements.empty())
         {
            RasterElement* pCube = dynamic_cast<RasterElement*>(importedElements.front());
            Service<ModelServices>()->setElementParent(pCube, dynamic_cast<DataElement*>(this));
            if (pCube != NULL)
            {
               clear();
               mpOdre.reset(pCube);
               DynamicObject* pMetadata = getMetadata();
               if (pMetadata != NULL)
               {
                  string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
                     CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
                  mOriginalAbscissa = dv_cast<vector<double> >(
                     pMetadata->getAttributeByPath(pCenterPath), vector<double>());

                  vector<string> sigNames = 
                     dv_cast<vector<string> >(pMetadata->getAttribute("Signature Names"), vector<string>());

                  SignatureLibrary* pLib = dynamic_cast<SignatureLibrary*>(this);
                  VERIFY(pLib != NULL);

                  unsigned int numSigs = pCubeDescriptor->getRowCount();

                  mSignatures.reserve(numSigs);
                  for (unsigned int i = 0; i < numSigs; ++i)
                  {
                     string name;
                     if (i >= sigNames.size())
                     {
                        stringstream stream;
                        stream << "Signature " << i+1;
                        name = stream.str();
                     }
                     else
                     {
                        name = sigNames[i];
                     }
                     DataDescriptor* pDataDesc =
                        Service<ModelServices>()->createDataDescriptor(name, "DataElement", pLib);
                     DataDescriptorImp* pSigDesc = dynamic_cast<DataDescriptorImp*>(pDataDesc);
                     mSignatures.push_back(new LibrarySignatureAdapter(*pSigDesc,
                        SessionItemImp::generateUniqueId(), i, pLib));
                     mSignatureNames[name] = mSignatures.back();
                  }
               }
               notify(SIGNAL_NAME(Subject, Modified));
               return true;
            }
         }
      }
   }

   return false;
}

bool SignatureLibraryImp::insertSignature(Signature* pSignature)
{
   vector<Signature*> sigs;
   sigs.push_back(pSignature);
   return insertSignatures(sigs);
}

bool SignatureLibraryImp::insertSignatures(const vector<Signature*>& signatures)
{
   if (find(signatures.begin(), signatures.end(), static_cast<Signature*>(NULL)) != signatures.end())
   {
      return false;
   }

   SignatureLibrary* pInterface = dynamic_cast<SignatureLibrary*>(this);
   if (!mOriginalAbscissa.empty())
   {
      return false;
   }

   const DynamicObject* pMetadata = getMetadata();
   VERIFY(pMetadata != NULL);
   const vector<double>* pWavelengths =
      dv_cast<vector<double> >(&pMetadata->getAttributeByPath(CENTER_WAVELENGTHS_METADATA_PATH));
   if (pWavelengths == NULL || pWavelengths->empty())
   {
      return false;
   }

   RasterDataDescriptor* pRasterDescriptor = RasterUtilities::generateRasterDataDescriptor(getName(), 
      pInterface, signatures.size(), pWavelengths->size(), 1, BIP, FLT8BYTES, ON_DISK);
   if (pRasterDescriptor == NULL)
   {
      return false;
   }

   ModelResource<RasterElement> pRasterElement (dynamic_cast<RasterElement*>(
      Service<ModelServices>()->createElement(pRasterDescriptor)));

   if (pRasterElement.get() == NULL)
   {
      return false;
   }

   PlugInResource resampler("Resampler");
   Resampler* pResampler = dynamic_cast<Resampler*>(resampler.get());
   if (pResampler == NULL)
   {
      return false;
   }

   DataAccessor accessor = pRasterElement->getDataAccessor();
   vector<Signature*>::const_iterator ppSignature;
   int i = 0;
   for (ppSignature = signatures.begin(); ppSignature != signatures.end(); ++ppSignature, ++i)
   {
      if (!accessor.isValid())
      {
         return false;
      }
      double* pRasterData = reinterpret_cast<double*>(accessor->getRow());
      const vector<double>* pFromData = dv_cast<vector<double> >(&(*ppSignature)->getData("Reflectance"));
      const vector<double>* pFromWavelengths = dv_cast<vector<double> >(&(*ppSignature)->getData("Wavelength"));
      if (pFromData == NULL || pFromWavelengths == NULL)
      {
         return false;
      }
      vector<double> toData;
      vector<int> toBands;
      vector<double> toFwhm;
      string errorMessage;
      bool success = pResampler->execute(*pFromData, toData, *pFromWavelengths, *pWavelengths, toFwhm,
         toBands, errorMessage);
      if (success == false || toBands.size() != pWavelengths->size())
      {
         return false;
      }
      std::copy(toData.begin(), toData.end(), pRasterData);
      accessor->nextRow();
      string name = (*ppSignature)->getName();
      DataDescriptor* pDataDesc = Service<ModelServices>()->createDataDescriptor(name, "DataElement", pInterface);
      DataDescriptorImp* pSigDesc = dynamic_cast<DataDescriptorImp*>(pDataDesc);
      mSignatures.push_back(new LibrarySignatureAdapter(*pSigDesc, SessionItemImp::generateUniqueId(), i, pInterface));
      mSignatureNames[name] = mSignatures.back();
   }

   mpOdre.reset(pRasterElement.release());

   mOriginalAbscissa = *pWavelengths;

   notify(SIGNAL_NAME(Subject, Modified));

   return true;
}

unsigned int SignatureLibraryImp::getNumSignatures() const
{
   return mSignatures.size();
}

bool SignatureLibraryImp::hasSignature(Signature* pSignature) const
{
   for (vector<LibrarySignatureAdapter*>::const_iterator pItem = mSignatures.begin();
      pItem != mSignatures.end(); ++pItem)
   {
      if (*pItem == pSignature)
      {
         return true;
      }
   }
   return false;
}

vector<Signature*> SignatureLibraryImp::getSignatures() const
{
   vector<Signature*> sigs;
   for (vector<LibrarySignatureAdapter*>::const_iterator pItem = mSignatures.begin();
      pItem != mSignatures.end(); ++pItem)
   {
      sigs.push_back(const_cast<LibrarySignatureAdapter*>(*pItem));
   }
   return sigs;
}

bool SignatureLibraryImp::removeSignature(Signature* pSignature, bool bDelete)
{
   if (mSignatures.size() == 1 && hasSignature(pSignature))
   {
      clear();
      return true;
   }
   else
   {
      return false;
   }
}

bool SignatureLibraryImp::removeSignatures(const vector<Signature*>& signatures, bool bDelete)
{
   if (mSignatures.size() != signatures.size())
   {
      return false;
   }

   // This is O(NlogN). A simple linear search over mSignatures for each sig in
   // signatures would be O(N*N). Even though there is overhead in creating the
   // map, this is prudent because the whole point of SignatureLibrary is to
   // provide good performance with large N.
   map<string, Signature*> sigNames;
   for (vector<Signature*>::const_iterator pItem = signatures.begin();
      pItem != signatures.end(); ++pItem)
   {
      if (*pItem != NULL)
      {
         sigNames[(*pItem)->getName()] = *pItem;
      }
   }

   if (sigNames == mSignatureNames)
   {
      clear();
      return true;
   }
   else
   {
      return false;
   }
}

template<class T>
struct Deleter
{
   void operator()(T* pT)
   {
      delete pT;
   }
};
void SignatureLibraryImp::clear(bool bDelete)
{
   desample();
   mOriginalAbscissa.clear();
   for_each(mSignatures.begin(), mSignatures.end(), Deleter<LibrarySignatureAdapter>());
   mSignatures.clear();
   mSignatureNames.clear();
   if (mpOdre.get() != NULL)
   {
      VERIFYNR(Service<ModelServices>()->destroyElement(mpOdre.get()));
   }
}

DataElement* SignatureLibraryImp::copy(const string& name, DataElement* pParent) const
{
   return NULL;
}

bool SignatureLibraryImp::serialize(SessionItemSerializer& serializer) const
{
   return false;
}

bool SignatureLibraryImp::deserialize(SessionItemDeserializer &deserializer)
{
   return false;
}

bool SignatureLibraryImp::toXml(XMLWriter* pXml) const
{
   return false;
}

bool SignatureLibraryImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   return false;
}

const string& SignatureLibraryImp::getObjectType() const
{
   static string sType("SignatureLibraryImp");
   return sType;
}

bool SignatureLibraryImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureLibrary"))
   {
      return true;
   }

   return SignatureSetImp::isKindOf(className);
}

bool SignatureLibraryImp::isKindOfElement(const string& className)
{
   if ((className == "SignatureLibraryImp") || (className == "SignatureLibrary"))
   {
      return true;
   }

   return SignatureSetImp::isKindOfElement(className);
}

void SignatureLibraryImp::ordinateDeleted(Subject &subject, const string &signal, const boost::any &data)
{
   clear();
}

void SignatureLibraryImp::ordinateModified(Subject &subject, const string &signal, const boost::any &data)
{
   mNeedToResample = true;
}
