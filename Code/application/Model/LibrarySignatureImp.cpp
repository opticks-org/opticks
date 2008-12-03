/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "ModelServicesImp.h"
#include "Signature.h"
#include "LibrarySignatureImp.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SignatureLibrary.h"
#include "UnitsImp.h"

#include <set>

using namespace std;

LibrarySignatureImp::LibrarySignatureImp(const DataDescriptorImp& descriptor, const string& id,
                                         unsigned int index, const SignatureLibrary* pLib) :
   SignatureImp(descriptor, id),
   mSignatureIndex(index),
   mpLibrary(pLib)
{
}

LibrarySignatureImp::~LibrarySignatureImp()
{
}

DataElement* LibrarySignatureImp::copy(const string& name, DataElement* pParent) const
{
   return NULL;
}

const string& LibrarySignatureImp::getObjectType() const
{
   static string sType("LibrarySignatureImp");
   return sType;
}

bool LibrarySignatureImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LibrarySignature"))
   {
      return true;
   }

   return SignatureImp::isKindOf(className);
}

bool LibrarySignatureImp::isKindOfElement(const string& className)
{
   if ((className == "LibrarySignatureImp") || (className == "LibrarySignature"))
   {
      return true;
   }

   return SignatureImp::isKindOfElement(className);
}

void LibrarySignatureImp::getElementTypes(vector<string>& classList)
{
   classList.push_back("LibrarySignature");
   SignatureImp::getElementTypes(classList);
}

const DataVariant& LibrarySignatureImp::getData(string name) const
{
   if (name == "Reflectance")
   {
      static DataVariant sReflectance;
      sReflectance = DataVariant();

      if (mpLibrary != NULL)
      {
         const double* pReflectance = mpLibrary->getOrdinateData(mSignatureIndex);
         if (pReflectance != NULL)
         {
            sReflectance = vector<double>(&pReflectance[0], &pReflectance[mpLibrary->getAbscissa().size()]);
         }
      }

      return sReflectance;
   }
   else if (name == "Wavelength")
   {
      static DataVariant sWavelength;
      if (mpLibrary == NULL)
      {
         sWavelength = DataVariant();
      }
      else
      {
         sWavelength = mpLibrary->getAbscissa();
      }

      return sWavelength;
   }

   return SignatureImp::getData(name);
}

void LibrarySignatureImp::setData(string name, const DataVariant &data)
{
   if (name == "Reflectance" || name == "Wavelength")
   {
      return;
   }
   SignatureImp::setData(name, data);
}

const Units* LibrarySignatureImp::getUnits(string name) const
{
   if (name == "Reflectance")
   {
      const Units* pUnits = NULL;
      if (mpLibrary != NULL)
      {
         const RasterElement* pRaster = mpLibrary->getOriginalOrdinateData();
         if (pRaster != NULL)
         {
            const RasterDataDescriptor* pDesc = dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
            if (pDesc != NULL)
            {
               pUnits = pDesc->getUnits();
            }
         }
      }

      return pUnits;
   }

   return SignatureImp::getUnits(name);
}
