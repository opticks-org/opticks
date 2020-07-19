/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "Signature.h"
#include "SignatureDataDescriptor.h"
#include "SignatureFileDescriptor.h"
#include "TestSuiteNewSession.h"
#include "TypeConverter.h"
#include "Units.h"

#include <string>

class SignatureUnitsTestCase : public TestCase
{
public:
   SignatureUnitsTestCase() : TestCase("SignatureUnits") {}
   bool run()
   {
      bool success(true);
      Signature* pSig = dynamic_cast<Signature*>(Service<ModelServices>()->createElement(
         "Test Signature", TypeConverter::toString<Signature>(), NULL));
      issearf(pSig != NULL);
      SignatureDataDescriptor* pSigDesc = dynamic_cast<SignatureDataDescriptor*>(pSig->getDataDescriptor());
      issearf(pSigDesc != NULL);
      std::string compName("Reflectance");
      const Units* pSigUnits = pSigDesc->getUnits(compName);  // pSig should not have any Units set
      issea(pSigUnits == NULL);

      // set up units values
      std::string unitName("Test");
      UnitType unitType(REFLECTANCE);
      double unitScale(0.0001);
      double unitRangeMin(-0.1);
      double unitRangeMax(1.1);
      double closeEnough(0.000001);

      // scope resources
      {
         FactoryResource<Units> pUnits;
         issearf(pUnits.get() != NULL);
         pUnits->setUnitName(unitName);
         pUnits->setUnitType(unitType);
         pUnits->setScaleFromStandard(unitScale);
         pUnits->setRangeMin(unitRangeMin);
         pUnits->setRangeMax(unitRangeMax);
         pSigDesc->setUnits(compName, pUnits.get());
         FactoryResource<SignatureFileDescriptor> pSigFd;
         issearf(pSigFd.get() != NULL);
         pSigFd->setUnits(compName, pUnits.get());
         pSigDesc->setFileDescriptor(pSigFd.get());
      }

      // check signature data descriptor units
      const Units* pDescUnits = pSigDesc->getUnits(compName);
      issea(pDescUnits != NULL);
      issea(pDescUnits->getUnitName() == unitName);
      issea(pDescUnits->getUnitType() == unitType);
      issea(abs(pDescUnits->getScaleFromStandard() - unitScale) < closeEnough);
      issea(abs(pDescUnits->getRangeMin() - unitRangeMin) < closeEnough);
      issea(abs(pDescUnits->getRangeMax() - unitRangeMax) < closeEnough);

      // check signature file descriptor units
      SignatureFileDescriptor* pSigFileDesc = dynamic_cast<SignatureFileDescriptor*>(pSigDesc->getFileDescriptor());
      issearf(pSigFileDesc != NULL);
      const Units* pFileUnits = pSigFileDesc->getUnits(compName);
      issearf(pFileUnits != NULL);
      issea(pFileUnits->getUnitName() == unitName);
      issea(pFileUnits->getUnitType() == unitType);
      issea(abs(pFileUnits->getScaleFromStandard() - unitScale) < closeEnough);
      issea(abs(pFileUnits->getRangeMin() - unitRangeMin) < closeEnough);
      issea(abs(pFileUnits->getRangeMax() - unitRangeMax) < closeEnough);

      return success;
   }
};

class SignatureTestSuite : public TestSuiteNewSession
{
public:
   SignatureTestSuite() : TestSuiteNewSession("Signature")
   {
      addTestCase(new SignatureUnitsTestCase);
   }
};

REGISTER_SUITE(SignatureTestSuite)

