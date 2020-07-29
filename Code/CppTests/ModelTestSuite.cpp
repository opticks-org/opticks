/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AoiElementAdapter.h"
#include "ApplicationServices.h"
#include "assert.h"
#include "ConnectionManager.h"
#include "DataElementGroup.h"
#include "DataVariant.h"
#include "DataVariantAnyData.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "EnumWrapper.h"
#include "GcpListAdapter.h"
#include "Layer.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ObjectFactoryImp.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "Signature.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "TiePointList.h"
#include "TypeConverter.h"
#include "UtilityServicesImp.h"

#include <string>
#include <vector>
using namespace std;

namespace
{
   enum CubeTypeEnum { ONE_PIXEL_CUBE, UNIFORM_CUBE, FULL_CUBE };

   /**
    * @EnumWrapper ::CubeTypeEnum.
    */
   typedef EnumWrapper<CubeTypeEnum> CubeType;
   RasterElement *buildCube( string pName, CubeType type )
   {
      //generate 1 band dimension with original number of 0
      //generate 1 row, 1 column if ONE_PIXEL_CUBE otherwise 2 row, 2 column
      int rowCount = (type == ONE_PIXEL_CUBE ? 1 : 2);
      int columnCount = (type == ONE_PIXEL_CUBE ? 1 : 2);
      RasterElement* pRaster = RasterUtilities::createRasterElement(pName, rowCount, columnCount, INT4SBYTES); 
      if (pRaster == NULL)
      {
         return NULL;
      }

      int *pData = reinterpret_cast<int*>(pRaster->getRawData());
      if (pData == NULL)
      {
         return pRaster;
      }

      switch( type )
      {
      case ONE_PIXEL_CUBE:
         pData[0] = 3;
         break;
      case UNIFORM_CUBE:
         pData[0] = pData[1] = pData[2] = pData[3] = 3;
         break;
      case FULL_CUBE:
         pData[0] = pData[1] = pData[2] = 0;
         pData[3] = 255;
         break;
      }

      return pRaster;
   }
};

class BandClassStatisticsTestCase : public TestCase
{
public:
   BandClassStatisticsTestCase() : TestCase("BandStatistics") {}
   bool run()
   {
      bool success = true;
      Service<ModelServices> pModel;
      vector<DimensionDescriptor> allBands;
      RasterDataDescriptor *pDescriptor = NULL;

      RasterElement* pCube = buildCube( "One Pixel Cube", ONE_PIXEL_CUBE );
      pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pCube->getDataDescriptor() );
      issea( pDescriptor != NULL );

      allBands = pDescriptor->getBands();
      issea( allBands.size() == 1 );
      issea( pCube->getStatistics( allBands[0] )->getMin() == 3.0 );
      issea( pCube->getStatistics( allBands[0] )->getMax() == 3.0 );
      issea( pCube->getStatistics( allBands[0] )->getAverage() == 3.0 );
      issea( pCube->getStatistics( allBands[0] )->getStandardDeviation() == 0 );
      pModel->destroyElement( pCube );

      pCube = buildCube( "Uniform Cube", UNIFORM_CUBE );
      pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pCube->getDataDescriptor() );
      issea( pDescriptor != NULL );

      allBands.clear();
      allBands = pDescriptor->getBands();
      issea( allBands.size() != 0 );
      issea( pCube->getStatistics( allBands[0] )->getMin() == 3.0 );
      issea( pCube->getStatistics( allBands[0] )->getMax() == 3.0 );
      issea( pCube->getStatistics( allBands[0] )->getAverage() == 3.0 );
      issea( pCube->getStatistics( allBands[0] )->getStandardDeviation() == 0 );
      pModel->destroyElement( pCube );

      pCube = buildCube( "Full Cube", FULL_CUBE );
      pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pCube->getDataDescriptor() );
      issea( pDescriptor != NULL );

      allBands.clear();
      allBands = pDescriptor->getBands();
      issea( allBands.size() != 0 );
      issea( pCube->getStatistics( allBands[0] )->getMin() == 0.0 );
      issea( pCube->getStatistics( allBands[0] )->getMax() == 255.0 );
      issea( pCube->getStatistics( allBands[0] )->getAverage() == 255.0 / 4.0 );
      issea( pCube->getStatistics( allBands[0] )->getStandardDeviation() == 255.0 / 2.0 );
      if( success )
      {
         const unsigned int *pBinCounts = NULL;
         const double *pBinCenters = NULL;
         pCube->getStatistics( allBands[0] )->getHistogram( pBinCenters, pBinCounts );
         issea( pBinCounts != NULL );
         issea( pBinCenters != NULL );
         issea( pBinCounts[0] == 3 );
         issea( pBinCenters[0] == 0 );
         for( int i = 1; i < 254; ++i )
         {
            issea( pBinCounts[i] == 0 );
            issea( pBinCenters[i] == i );
         }
         issea( pBinCounts[255] == 1 );
         issea( pBinCenters[255] == 255 );
      }
      pModel->destroyElement( pCube );

      return success;
   }
};

class SessionTestCase : public TestCase
{
public:
   SessionTestCase() : TestCase("Session") {}
   bool run()
   {
      bool success = true;
      Service<ModelServices> pModel;
      RasterElement *pFullCube = buildCube( "Full Cube", FULL_CUBE );
      RasterElement *pUniformCube = buildCube( "Uniform Cube", UNIFORM_CUBE );

      issea( pModel->getElement( "Full Cube", TypeConverter::toString<RasterElement>(), NULL ) == pFullCube );
      issea( pModel->getElement( "Full Cube", "RasterElementAdapter", NULL ) == pFullCube );
      issea( pModel->getElement( "Uniform Cube", TypeConverter::toString<RasterElement>(), NULL ) == pUniformCube );
      issea( pModel->getElement( "Uniform Cube", "RasterElementAdapter", NULL ) == pUniformCube );

      AoiElement* pAoi = dynamic_cast<AoiElement*>(pModel->createElement("AoiName", TypeConverter::toString<AoiElement>(), pFullCube));
      pModel->setElementName( pAoi, "AOI 1" );

      issea( pModel->getElement( "AOI 1", TypeConverter::toString<AoiElement>(), pFullCube ) == pAoi );
      issea( pModel->getElement( "AOI 1", TypeConverter::toString<AoiElement>(), pUniformCube ) == NULL );
      pModel->setElementName( pAoi, "AOI 2" );
      issea( pModel->getElement( "AOI 1", TypeConverter::toString<AoiElement>(), pFullCube ) == NULL );
      issea( pModel->getElement( "AOI 2", TypeConverter::toString<AoiElement>(), pFullCube ) == pAoi );

      AoiElement* pAoi3 = dynamic_cast<AoiElement*>(pModel->createElement("AoiName", TypeConverter::toString<AoiElement>(), pFullCube));
      pModel->setElementName( pAoi3, "AOI 3" );

      GcpList* pGcpList = dynamic_cast<GcpList*>(pModel->createElement("GcpName", TypeConverter::toString<GcpList>(), pUniformCube));
      pModel->setElementName( pGcpList, "GCPList 1" );

      vector<DataElement*> elements = pModel->getElements(pFullCube, TypeConverter::toString<AoiElement>());
      issea( elements.size() == 2 );
      issea( elements[0]->getName() == "AOI 2" );
      issea( elements[1]->getName() == "AOI 3" );
      elements.clear();
      elements = pModel->getElements(pFullCube, TypeConverter::toString<GcpList>());
      issea( elements.size() == 0 );
      elements.clear();
      elements = pModel->getElements(pUniformCube, TypeConverter::toString<GcpList>());
      issea( elements.size() == 1 );
      issea( elements[0]->getName() == "GCPList 1" );

      vector<string> elementNames;
      elementNames = pModel->getElementNames(pFullCube, TypeConverter::toString<AoiElement>());
      issea( elementNames.size() == 2 );
      issea( elementNames[0] == "AOI 2" );
      issea( elementNames[1] == "AOI 3" );

      issea( true == pModel->removeElement(pAoi) );
      issea( false == pModel->removeElement(pAoi) );
      issea( pModel->getElementNames(TypeConverter::toString<AoiElement>()).size() == 1);

      pModel->destroyElement(pUniformCube);

      issea( pModel->getElement("Uniform Cube", TypeConverter::toString<RasterElement>(), NULL ) == NULL );
      issea( pModel->getElement("Uniform Cube", "RasterElementAdapter", NULL ) == NULL );
      issea( pModel->getElementNames(TypeConverter::toString<GcpList>()).size() == 0 );

      pModel->clear();
      issea( pModel->getElement("Full Cube", TypeConverter::toString<RasterElement>(), NULL ) == NULL );
      issea( pModel->getElement("Full Cube", "RasterElementAdapter", NULL ) == NULL );
      issea( pModel->getElementNames(TypeConverter::toString<AoiElement>()).size() == 0);

      return success;
   }
};

class BitMaskTestCase : public TestCase
{
public:
   BitMaskTestCase() : TestCase("BitMask") {}

   bool testBitMask(const BitMaskImp& theMask, LocationType ul, LocationType lr, 
      int count)
   {
      bool success = true;
      int mbx1 = -1, mby1 = -1, mbx2 = -1, mby2 = -1;
      theMask.getMinimalBoundingBox(mbx1, mby1, mbx2, mby2);
      issea( ul.mX == mbx1 );
      issea( ul.mY == mby1 );
      issea( lr.mX == mbx2 );
      issea( lr.mY == mby2 );

      issea( theMask.getCount() == count );

      int bx1 = -1, by1 = -1, bx2 = -1, by2 = -1;
      theMask.getBoundingBox(bx1, by1, bx2, by2);

      if ( theMask.getCount() > 0)
      {
         //the minimal bounding box should be at most as large as the
         //normal bounding box
         issea( (mbx1 >= bx1) && (mby1 >= by1) && (mbx2 <= bx2) && (mby2 <= by2) );

         //iterate through the whole bounding box, not minimal and verify
         //that we getCount() pixels turned on.
         int pixelsOn = 0;
         for (int y = by1; y <= by2; y++)
         {
            for (int x = bx1; x <= bx2; x++)
            {
               if (theMask.getPixel(x,y))
               {
                  pixelsOn++;
               }
            }
         }
         issea(pixelsOn == theMask.getCount());
      }

      return success;
   }

   bool createTestedRectangularBitMask(BitMaskImp& mask, int length, bool isHoriz, int xOffset, int yOffset)
   {
      //create rectangular bit mask of width 10 centered at xoffset and yoffset.
      int width = 10;
      int xLength = length;
      int yLength = width;
      if (!isHoriz)
      {
         xLength = width;
         yLength = length;
      }
      int x1 = xOffset-(xLength/2);
      int y1 = yOffset-(yLength/2);
      int x2 = xOffset+(xLength/2);
      int y2 = yOffset+(yLength/2);

      mask.setRegion(x1, y1, x2, y2, DRAW);
      return testBitMask(mask, LocationType(x1,y1), LocationType(x2,y2), (x2-x1+1)*(y2-y1+1));
   }

   bool testXor(const BitMaskImp& bitMask1, const BitMaskImp& bitMask2, BitMaskImp& result)
   {
      //run xor, but in the process calculate it
      //using definition of A ^ B = !(A & B) & (A | B)

      BitMaskImp bitMaskXorDirect = bitMask1;
      bitMaskXorDirect ^= bitMask2;
      //!(A ^ B)
      BitMaskImp paren1 = bitMask1;
      paren1 &= bitMask2;
      paren1.invert();
      //A | B
      BitMaskImp paren2 = bitMask1;
      paren2 |= bitMask2;

      //now the & between parens
      BitMaskImp bitMaskXorCalc = paren1;
      bitMaskXorCalc &= paren2;

      result = bitMaskXorDirect;

      //verify both ways of calculating give the same result
      return bitMaskXorDirect.compare(bitMaskXorCalc);
   }

   bool run()
   {
      bool success = true;

      int OX = 1000, OY = 1000;
      //test default constructor
      {
         BitMaskImp bitMask;
         issea(testBitMask(bitMask, LocationType(0,0), LocationType(0,0), 0));
      }

      //test bounding box
      {
         //try one pixel bitmask, moving the x across all the bits           
         int i;
         for (i = 0; i <= 33; i++)
         {
            BitMaskImp bitMask;
            bitMask.setPixel(OX+i, OY, true);
            issea(testBitMask(bitMask, LocationType(OX+i,OY), LocationType(OX+i,OY), 1));
         }

         //try one pixel bitmask, moving the y across all the bits
         for (i = 0; i <= 33; i++)
         {
            BitMaskImp bitMask;
            bitMask.setPixel(OX,OY+i, true);
            issea(testBitMask(bitMask, LocationType(OX,OY+i), LocationType(OX,OY+i), 1));
         }
      }

      //test bounding box
      {
         BitMaskImp bitMask;
         bitMask.setPixel(OX, OY, true);
         issea(testBitMask(bitMask, LocationType(OX,OY), LocationType(OX,OY), 1));
         bitMask.setPixel(OX+1, OY, true);
         issea(testBitMask(bitMask, LocationType(OX,OY), LocationType(OX+1,OY), 2));
         bitMask.setPixel(OX, OY+2, true);
         issea(testBitMask(bitMask, LocationType(OX,OY), LocationType(OX+1,OY+2), 3)); 
         bitMask.setPixel(OX-1, OY, true);
         issea(testBitMask(bitMask, LocationType(OX-1,OY), LocationType(OX+1,OY+2), 4));
         bitMask.setPixel(OX, OY-2, true);
         issea(testBitMask(bitMask, LocationType(OX-1,OY-2), LocationType(OX+1,OY+2), 5));
         bitMask.setPixel(OX+500, OY+500, false);
         issea(testBitMask(bitMask, LocationType(OX-1,OY-2), LocationType(OX+1,OY+2), 5));
      }

      //test toggling pixel value manually
      {
         BitMaskImp bitMask;
         bitMask.setPixel(OX+1, OY+2, true);
         issea(testBitMask(bitMask, LocationType(OX+1,OY+2), LocationType(OX+1,OY+2), 1));
         bitMask.setPixel(OX+1, OY+2, false);
         issea(testBitMask(bitMask, LocationType(0,0), LocationType(0,0), 0));
      }

      //test setRegion here
      {
         BitMaskImp bitMask1;
         bitMask1.setRegion(OX-10, OY-20, OX+10, OY+20, DRAW);
         issea(testBitMask(bitMask1, LocationType(OX-10, OY-20), LocationType(OX+10, OY+20), 861));
         BitMaskImp bitMask2;
         bitMask2.setRegion(OX+10, OY+20, OX-10, OY-20, DRAW);
         issea(testBitMask(bitMask2, LocationType(OX-10, OY-20), LocationType(OX+10, OY+20), 861));
      }

      //test merging
      // + pattern
      {
         BitMaskImp bitMask1;
         issea(createTestedRectangularBitMask(bitMask1, 20, true, OX, OY));
         BitMaskImp bitMask2;
         issea(createTestedRectangularBitMask(bitMask2, 20, false, OX, OY));

         //OR mask's together
         BitMaskImp bitMaskOr = bitMask1;
         bitMaskOr.merge(bitMask2);
         //we now have a + pattern, verify         
         issea(testBitMask(bitMaskOr, LocationType(OX-10,OY-10), LocationType(OX+10,OY+10),
            bitMask1.getCount()*2-(11*11)/*total - overlap*/));
         //iterate and verify all pixels turned on correctly

         //AND mask's together
         BitMaskImp bitMaskAnd = bitMask1;
         bitMaskAnd.intersect(bitMask2);
         issea(testBitMask(bitMaskAnd, LocationType(OX-5,OY-5),LocationType(OX+5,OY+5),
            11*11/*just the middle section*/));

         //XOR mask's together
         BitMaskImp bitMaskXor;
         issea(testXor(bitMask1, bitMask2, bitMaskXor));
         issea(testBitMask(bitMaskXor, LocationType(OX-10,OY-10),LocationType(OX+10,OY+10),
            bitMask1.getCount()*2-(11*11*2)/*total-hole for intersect*/));
      }

      //test merging
      // ----
      // ---- pattern
      {
         BitMaskImp bitMask1;
         issea(createTestedRectangularBitMask(bitMask1, 20, true, OX, OY));
         BitMaskImp bitMask2;
         issea(createTestedRectangularBitMask(bitMask2, 20, true, OX, OY+20));

         //OR mask's together
         BitMaskImp bitMaskOr = bitMask1;
         bitMaskOr.merge(bitMask2);
         issea(testBitMask(bitMaskOr, LocationType(OX-10,OY-5), LocationType(OX+10, OY+25),
            bitMask1.getCount()*2/*total*/));
         //iterate and verify all pixels turned on correctly

         //AND mask's together
         BitMaskImp bitMaskAnd = bitMask1;
         bitMaskAnd.intersect(bitMask2);
         issea(testBitMask(bitMaskAnd, LocationType(0,0), LocationType(0,0), 0));

         //XOR mask's together
         BitMaskImp bitMaskXor;
         issea(testXor(bitMask1, bitMask2, bitMaskXor));
         issea(testBitMask(bitMaskXor, LocationType(OX-10,OY-5), LocationType(OX+10, OY+25),
            bitMask1.getCount()*2/*total*/));
      }

      //test pass thru
      //create two square bitmask's and pass one diagonally over the other
      {
         BitMaskImp bitMask1;
         bitMask1.setRegion(OX,OY,OX+33,OY+33,DRAW);

         BitMaskImp bitMaskAnd = bitMask1;
         BitMaskImp bitMaskOr = bitMask1;
         BitMaskImp bitMaskXor = bitMask1;
         //bitMask1 stays put
         //bitMask2 moves diagonally
         unsigned int numFailed = AssertionCounter::getNumFailed();
         for (int count = 0; count < 33; count++)
         {
            BitMaskImp bitMask2;
            bitMask2.setRegion(OX+count,OY+count,OX+count+33,OY+count+33,DRAW);

            //AND and verify
            bitMaskAnd.intersect(bitMask2);
            assert(testBitMask(bitMaskAnd, LocationType(OX+count,OY+count),
               LocationType(OX+33,OY+33),(34-count)*(34-count)));

            //OR and verify
            bitMaskOr.merge(bitMask2);
            assert(testBitMask(bitMaskOr, LocationType(OX,OY),
               LocationType(OX+33+count,OY+33+count),
               (34*34*2)/*two squares*/-((34-count)*(34-count))/*minus overlap*/));

            //XOR and verify
            //use testXor here
            bitMaskXor.toggle(bitMask2);
            if (count == 0)
            {
               //special case where squares completely overlap
               assert(testBitMask(bitMaskXor, LocationType(0,0),LocationType(0,0),0));
            }
            else
            {
               assert(testBitMask(bitMaskXor, LocationType(OX,OY),
                  LocationType(OX+33+count,OY+33+count),
                  (34*34*2)/*two squares*/-((34-count)*(34-count)*2)/*minus intersect*/));
            }

            //undo AND and verify
            bitMaskAnd |= bitMask1; //ie. (A & B) | A = A
            assert(bitMask1.compare(bitMaskAnd));

            //undo OR and verify
            bitMaskOr &= bitMask1; //ie. (A | B) & A = A
            assert(bitMask1.compare(bitMaskOr));

            //undo XOR and verify
            bitMaskXor ^= bitMask2; //ie. (A ^ B) ^ B = A
            assert(bitMask1.compare(bitMaskXor));
         }
         if (AssertionCounter::getNumFailed() > numFailed)
         {
            success = false;
         }
      }

      return success;
   }
};

class BitMaskRegionConstructorTestCase : public TestCase
{
public:
   BitMaskRegionConstructorTestCase() : TestCase("BitMaskRegionConstructor") {}
   bool run()
   {
      bool success = true;
      bool aRegion[9] = {false};
      bool *aaRegion[3];
      bool **pRegion = NULL;

      aRegion[2] = true;
      aRegion[3] = true;
      aRegion[5] = true;
      aRegion[7] = true;
      aaRegion[0] = &aRegion[0];
      aaRegion[1] = &aRegion[3];
      aaRegion[2] = &aRegion[6];

      pRegion = static_cast<bool**>( aaRegion );

      BitMaskImp imp( const_cast<const bool **>( pRegion ), 10, 20, 12, 22 );

      for( int x = 10; x <= 12; ++x )
      {
         for( int y = 20; y <= 22; ++y )
         {
            bool pix = imp.getPixel( x, y );
            issea( pix == aaRegion[y - 20][x - 10] );
         }
      }

      return success;
   }
};

class RenameAndReparentDataElement : public TestCase
{
public:
   RenameAndReparentDataElement() : TestCase("RenameAndReparent") {}
   bool run()
   {
      bool success = true;

      ModelResource<AoiElement> pAoi1("RenameAndReparent1", NULL, "AoiElement");
      ModelResource<AoiElement> pAoi2("RenameAndReparent2", NULL, "AoiElement");
      ModelResource<AoiElement> pAoi2Child("RenameAndReparent1", pAoi2.get(), "AoiElement");

      issea(pAoi1.get() != NULL);
      issea(pAoi2.get() != NULL);
      issea(pAoi2Child.get() != NULL);

      Service<ModelServices> pModel;
      issea(pModel.get() != NULL);

      if (success)
      {
         // test failures:
         issea(pModel->setElementName(pAoi1.get(), "RenameAndReparent2") == false);
         issea(pModel->setElementParent(pAoi1.get(), pAoi2.get()) == false);

         issea(pModel->getElement("RenameAndReparent1", "AoiElement", NULL) == pAoi1.get());

         // test successes:
         issea(pModel->setElementName(pAoi2Child.get(), "RenameAndReparent3"));
         issea(pAoi2Child->getName() == "RenameAndReparent3");
         issea(pModel->getElement("RenameAndReparent3", "AoiElement", pAoi2.get()) == pAoi2Child.get()); // new position
         issea(pModel->getElement("RenameAndReparent1", "AoiElement", pAoi2.get()) == NULL); // old position

         issea(pModel->setElementParent(pAoi2Child.get(), NULL));
         issea(pAoi2Child->getParent() == NULL);
         issea(pModel->getElement("RenameAndReparent3", "AoiElement", NULL) == pAoi2Child.get()); // new position
         issea(pModel->getElement("RenameAndReparent3", "AoiElement", pAoi2.get()) == NULL); // old position
      }

      return success;
   }
};

class ModelResourceGetTest : public TestCase
{
public:
   ModelResourceGetTest() : TestCase("ModelResourceGetTest") {}
   bool run()
   {
      bool success = true;

      Service<ModelServices> pModel;     

      string fileName = TestUtilities::getTestDataPath() + "tipjul5bands.sio";

      ModelResource<GcpList>pList( "Hello" );
      DataElement* pElement = NULL;
      pElement = pModel->getElement( "Hello", "GcpList", NULL );
      issea( pElement != NULL );

      return success;
   }
};

class DataElementGroupTest : public TestCase
{
public:
   DataElementGroupTest() : TestCase("DataElementGroup") {}
   bool run()
   {
      bool success = true;
      Service<ModelServices> pModel;

      AoiElement* pAoi1 = NULL;
      pAoi1 = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement( "testAoi1", "AoiElement", NULL ) );
      issea( pAoi1 != NULL );

      AoiElement* pAoi2 = NULL;
      pAoi2 = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement( "testAoi2", "AoiElement", NULL ) );
      issea( pAoi2 != NULL );

      GcpList* pList1 = NULL;
      pList1 = static_cast<GcpList*>(ModelServicesImp::instance()->createElement( "testGcpList1", "GcpList", NULL ) );
      issea( pList1 != NULL );

      GcpList* pList2 = NULL;
      pList2 = static_cast<GcpList*>(ModelServicesImp::instance()->createElement( "testGcpList2", "GcpList", NULL ) );
      issea( pList2 != NULL );

      TiePointList* pTiePointList1 = NULL;
      pTiePointList1 = static_cast<TiePointList*>(ModelServicesImp::instance()->createElement(
         "testTiePointList1", "TiePointList", NULL ) );
      issea( pTiePointList1 != NULL );

      TiePointList* pTiePointList2 = NULL;
      pTiePointList2 = static_cast<TiePointList*>(ModelServicesImp::instance()->createElement(
         "testTiePointList2", "TiePointList", NULL ) );
      issea( pTiePointList2 != NULL );

      DataElementGroup *pGroup = NULL;
      pGroup = dynamic_cast<DataElementGroup*>( pModel->createElement( "TestGroupElement", "DataElementGroup", NULL ) );
      issea( pGroup != NULL );

      issea( pGroup->getNumElements() == 0 );
      issea( pGroup->isUniform() == true );
      issea( pGroup->enforceUniformity( false ) == true );

      // start adding elements to the group
      issea( pGroup->insertElement( pAoi1 ) == true );
      issea( pGroup->isUniform() == true );
      issea( pGroup->getNumElements() == 1 );

      issea( pGroup->insertElement( pAoi2 ) == true );
      issea( pGroup->isUniform() == true );
      issea( pGroup->getNumElements() == 2 );

      issea( pGroup->insertElement( pList1 ) == true );
      issea( pGroup->isUniform() == false );
      issea( pGroup->getNumElements() == 3 );

      issea( pGroup->insertElement( pList2 ) == true );
      issea( pGroup->isUniform() == false );
      issea( pGroup->getNumElements() == 4 );

      issea( pGroup->insertElement( pTiePointList1 ) == true );
      issea( pGroup->isUniform() == false );
      issea( pGroup->getNumElements() == 5 );

      issea( pGroup->insertElement( pTiePointList2 ) == true );
      issea( pGroup->isUniform() == false );
      issea( pGroup->getNumElements() == 6 );

      // you should not be able to enforce uniformity if the group is already not uniform
      issea( pGroup->enforceUniformity( true ) == false );

      // check the contents of the group
      issea( pGroup->hasElement( pAoi1 ) == true );
      issea( pGroup->hasElement( pAoi2 ) == true );
      issea( pGroup->hasElement( pList1 ) == true );
      issea( pGroup->hasElement( pList2 ) == true );
      issea( pGroup->hasElement( pTiePointList1 ) == true );
      issea( pGroup->hasElement( pTiePointList2 ) == true );

      // remove some of the elements from the group
      issea( pGroup->removeElement( pAoi2, false ) == true );
      issea( pGroup->removeElement( pList2, false ) == true );

      // check the contents of the group again
      issea( pGroup->hasElement( pAoi1 ) == true );
      issea( pGroup->hasElement( pAoi2 ) == false );
      issea( pGroup->hasElement( pList1 ) == true );
      issea( pGroup->hasElement( pList2 ) == false );
      issea( pGroup->hasElement( pTiePointList1 ) == true );
      issea( pGroup->hasElement( pTiePointList2 ) == true );
      issea( pGroup->isUniform() == false );
      issea( pGroup->getNumElements() == 4 );
      issea( pGroup->enforceUniformity( true ) == false );

      // remove some more elements from the group
      issea( pGroup->removeElement( pAoi1, false ) == true );
      issea( pGroup->removeElement( pList1, false ) == true );
      issea( pGroup->isUniform() == true );
      issea( pGroup->getNumElements() == 2 );
      issea( pGroup->enforceUniformity( true ) == true );

      issea( pGroup->removeElement( pTiePointList1, false ) == true );

      // now that uniformity is enforced, only TiePointLists should be allowed
      issea( pGroup->insertElement( pAoi1 ) == false );
      issea( pGroup->insertElement( pList1 ) == false );
      issea( pGroup->insertElement( pTiePointList1 ) == true );

      // empty the group
      pGroup->clear( false );
      issea( pGroup->isUniform() == true );
      issea( pGroup->getNumElements() == 0 );
      issea( pGroup->enforceUniformity( false ) == true );

      // make sure the clear() did not remove any of the elements
      issea( pModel->getElement( "testAoi1", "AoiElement", NULL ) != NULL );
      issea( pModel->getElement( "testAoi2", "AoiElement", NULL ) != NULL );
      issea( pModel->getElement( "testGcpList1", "GcpList", NULL ) != NULL );
      issea( pModel->getElement( "testGcpList2", "GcpList", NULL ) != NULL );
      issea( pModel->getElement( "testTiePointList1", "TiePointList", NULL ) != NULL );
      issea( pModel->getElement( "testTiePointList2", "TiePointList", NULL ) != NULL );

      vector<DataElement*> elementVector;
      elementVector.push_back( pAoi1 );
      elementVector.push_back( pAoi2 );
      elementVector.push_back( pList1 );
      elementVector.push_back( pList2 );
      elementVector.push_back( pTiePointList1 );
      elementVector.push_back( pTiePointList2 );
      issea( elementVector.size() == 6 );

      // try to add a vector of DataElements to the group
      issea( pGroup->insertElements( elementVector ) == true );
      issea( pGroup->isUniform() == false );
      issea( pGroup->getNumElements() == 6 );

      // check the contents of the group
      issea( pGroup->hasElement( pAoi1 ) == true );
      issea( pGroup->hasElement( pAoi2 ) == true );
      issea( pGroup->hasElement( pList1 ) == true );
      issea( pGroup->hasElement( pList2 ) == true );
      issea( pGroup->hasElement( pTiePointList1 ) == true );
      issea( pGroup->hasElement( pTiePointList2 ) == true );

      elementVector.clear();
      elementVector.push_back( pAoi1 );
      elementVector.push_back( pList1 );
      elementVector.push_back( pTiePointList1 );

      // remove some of the elements from the group
      issea( pGroup->removeElements( elementVector, false ) == true );
      issea( pGroup->isUniform() == false );
      issea( pGroup->getNumElements() == 3 );

      vector<DataElement*> returnedElements = pGroup->getElements();
      issea( returnedElements.size() == 3 );
      issea( returnedElements.at( 0 )->getName() == pAoi2->getName() );
      issea( returnedElements.at( 1 )->getName() == pList2->getName() );
      issea( returnedElements.at( 2 )->getName() == pTiePointList2->getName() );

      pGroup->clear( false );
      issea( pGroup->isUniform() == true );
      issea( pGroup->getNumElements() == 0 );

      issea( pGroup->enforceUniformity( true ) == true );
      issea( pGroup->insertElement( pAoi1 ) == true );
      issea( pGroup->insertElement( pList1 ) == false );
      issea( pGroup->insertElement( pAoi2 ) == true );      
      issea( pGroup->insertElement( pList2 ) == false );
      issea( pGroup->insertElement( pTiePointList1 ) == false );
      issea( pGroup->insertElement( pTiePointList2 ) == false );

      pGroup->clear( true );

      // make sure the clear() removed these elements
      issea( pModel->getElement( "testAoi1", "AoiElement", NULL ) == NULL );
      issea( pModel->getElement( "testAoi2", "AoiElement", NULL ) == NULL );

      // these elements were not in the group, so they should not have been deleted
      issea( pModel->getElement( "testGcpList1", "GcpList", NULL ) != NULL );
      issea( pModel->getElement( "testGcpList2", "GcpList", NULL ) != NULL );
      issea( pModel->getElement( "testTiePointList1", "TiePointList", NULL ) != NULL );
      issea( pModel->getElement( "testTiePointList2", "TiePointList", NULL ) != NULL );

      pModel->destroyElement( pGroup );
      pModel->destroyElement( pList1 );
      pModel->destroyElement( pList2 );
      pModel->destroyElement( pTiePointList1 );
      pModel->destroyElement( pTiePointList2 );
      return success;
   }
};

class SignatureDataTest : public TestCase
{
public:
   SignatureDataTest() : TestCase("SignatureData") {}
   bool run()
   {
      bool success = true;
      Service<ModelServices> pModel;

      Signature *pTestSignature = NULL;
      pTestSignature = dynamic_cast<Signature*>( pModel->createElement( "testSignature", "Signature", NULL ) );
      issea( pTestSignature != NULL );

      vector<int> evenVector;
      evenVector.push_back( 2 );
      evenVector.push_back( 4 );
      evenVector.push_back( 6 );
      evenVector.push_back( 8 );

      vector<int> oddVector;
      oddVector.push_back( 1 );
      oddVector.push_back( 3 );
      oddVector.push_back( 5 );
      oddVector.push_back( 7 );

      string dataString = "aTestString";
      double dataDouble = -12345.987;
      bool dataBool = true;

      // start adding data of various types
      pTestSignature->setData( "testData1", DataVariant( dataString ) );
      pTestSignature->setData( "testData2", DataVariant( evenVector ) );
      pTestSignature->setData( "testData3", DataVariant( dataDouble ) );
      pTestSignature->setData( "testData4", DataVariant( dataBool ) );

      // verify the data
      issea( pTestSignature->getData( "testData1" ).isValid() == true );
      issea( pTestSignature->getData( "testData2" ).isValid() == true );
      issea( pTestSignature->getData( "testData3" ).isValid() == true );
      issea( pTestSignature->getData( "testData4" ).isValid() == true );

      issea( *( pTestSignature->getData( "testData1" ).getPointerToValue<string>() ) == "aTestString" );
      vector<int> returnedVector = *( pTestSignature->getData( "testData2" ).getPointerToValue< vector<int> >() );
      issea( returnedVector.at( 0 ) == 2 );
      issea( returnedVector.at( 1 ) == 4 );
      issea( returnedVector.at( 2 ) == 6 );
      issea( returnedVector.at( 3 ) == 8 );
      issea( *( pTestSignature->getData( "testData3" ).getPointerToValue<double>() ) == -12345.987 );
      issea( *( pTestSignature->getData( "testData4" ).getPointerToValue<bool>() ) == true );

      // this should overwrite the previous vector because the name is the same
      pTestSignature->setData( "testData2", DataVariant( oddVector ) );
      returnedVector.clear();

      // verify the data again
      issea( pTestSignature->getData( "testData1" ).isValid() == true );
      issea( pTestSignature->getData( "testData2" ).isValid() == true );
      issea( pTestSignature->getData( "testData3" ).isValid() == true );
      issea( pTestSignature->getData( "testData4" ).isValid() == true );

      issea( *( pTestSignature->getData( "testData1" ).getPointerToValue<string>() ) == "aTestString" );
      returnedVector = *( pTestSignature->getData( "testData2" ).getPointerToValue< vector<int> >() );
      issea( returnedVector.at( 0 ) == 1 );
      issea( returnedVector.at( 1 ) == 3 );
      issea( returnedVector.at( 2 ) == 5 );
      issea( returnedVector.at( 3 ) == 7 );
      issea( *( pTestSignature->getData( "testData3" ).getPointerToValue<double>() ) == -12345.987 );
      issea( *( pTestSignature->getData( "testData4" ).getPointerToValue<bool>() ) == true );

      pModel->destroyElement( pTestSignature );

      return success;
   }
};

class CustomAnyData : public AnyData
{
public:
   CustomAnyData() :
      mInt(-1),
         mUnsignedInt(1),
         mDouble(0.0),
         mpPointer(NULL)
      {
         mClassCount++;
      }

      ~CustomAnyData()
      {
         mClassCount--;
      }

      AnyData* copy() const
      {
         CustomAnyData* pData = new CustomAnyData();
         if (pData != NULL)
         {
            pData->mInt = mInt;
            pData->mUnsignedInt = mUnsignedInt;
            pData->mDouble = mDouble;
            pData->mpPointer = mpPointer;
         }

         return pData;
      }

      int mInt;
      unsigned int mUnsignedInt;
      double mDouble;
      void* mpPointer;

      static int mClassCount;
};

int CustomAnyData::mClassCount = 0;

class AnyTestCase : public TestCase
{
public:
   AnyTestCase() : TestCase("Any") {}
   bool run()
   {
      bool success = true;
      Service<ModelServices> pModel;
      const string dataType = "Any";
      const string customDataType = "CustomAny";

      // Data variant data
      Any* pVariantAny = dynamic_cast<Any*>(pModel->createElement("DataVariantAnyName", dataType, NULL));
      issea(pVariantAny != NULL);
      issea(pVariantAny->isKindOf(dataType) == true);
      issea(CustomAnyData::mClassCount == 0);

      vector<DataElement*> anyElements = pModel->getElements(dataType);
      issea(anyElements.size() == 1);

      DataVariantAnyData* pVariantData = new DataVariantAnyData(12345);
      issea(pVariantData != NULL);
      pVariantAny->setData(pVariantData);

      Any* pAny = dynamic_cast<Any*>(pModel->getElement("DataVariantAnyName", dataType, NULL));
      issea(pAny == pVariantAny);
      issea(pAny->isKindOf(dataType) == true);

      int dataValue = 0;
      DataVariantAnyData* pAnyData = dynamic_cast<DataVariantAnyData*>(pAny->getData());
      issea(pAnyData->getAttribute().getValue(dataValue) == true);
      issea(dataValue == 12345);

      // Custom data
      Any* pCustomAny = dynamic_cast<Any*>(pModel->createElement("CustomAnyName", customDataType, NULL));
      issea(pCustomAny == NULL);
      issea(CustomAnyData::mClassCount == 0);

      pModel->addElementType(customDataType);

      pCustomAny = dynamic_cast<Any*>(pModel->createElement("CustomAnyName", customDataType, NULL));
      issea(pCustomAny != NULL);
      issea(pCustomAny->isKindOf(customDataType) == true);
      issea(CustomAnyData::mClassCount == 0);

      anyElements = pModel->getElements(customDataType);
      issea(anyElements.size() == 1);
      anyElements = pModel->getElements(dataType);
      issea(anyElements.size() == 2);

      CustomAnyData* pCustomData = new CustomAnyData();
      issea(pCustomData != NULL);
      issea(CustomAnyData::mClassCount == 1);

      pCustomData->mInt = -123;
      pCustomData->mUnsignedInt = 456;
      pCustomData->mDouble = 78.9;
      pCustomData->mpPointer = pCustomData;
      pCustomAny->setData(pCustomData);

      pAny = dynamic_cast<Any*>(pModel->getElement("CustomAnyName", dataType, NULL));
      issea(pAny != NULL);
      issea(pAny == pCustomAny);

      pAny = dynamic_cast<Any*>(pModel->getElement("CustomAnyName", customDataType, NULL));
      issea(pAny != NULL);
      issea(pAny == pCustomAny);

      pAny = model_cast<Any*>(pModel->getElement("CustomAnyName", customDataType, NULL));
      issea(pAny != NULL);
      issea(pAny == pCustomAny);

      pCustomData = dynamic_cast<CustomAnyData*>(pAny->getData());
      issea(pCustomData != NULL);

      pCustomData = model_cast<CustomAnyData*>(pModel->getElement("CustomAnyName", customDataType, NULL));
      issea(pCustomData != NULL);
      issea(pCustomData->mInt == -123);
      issea(pCustomData->mUnsignedInt == 456);
      issea(pCustomData->mDouble == 78.9);
      issea(pCustomData->mpPointer == pCustomData);

      // Copy
      Any* pCustomAnyCopy = static_cast<Any*>(pCustomAny->copy("CustomAnyCopy", NULL));
      issea(pCustomAnyCopy != NULL);
      issea(pCustomAnyCopy->isKindOf(customDataType) == true);
      issea(CustomAnyData::mClassCount == 2);

      anyElements = pModel->getElements(customDataType);
      issea(anyElements.size() == 2);
      anyElements = pModel->getElements(dataType);
      issea(anyElements.size() == 3);

      CustomAnyData* pCustomDataCopy = dynamic_cast<CustomAnyData*>(pCustomAnyCopy->getData());
      issea(pCustomDataCopy != NULL);
      issea(pCustomDataCopy->mInt == -123);
      issea(pCustomDataCopy->mUnsignedInt == 456);
      issea(pCustomDataCopy->mDouble == 78.9);
      issea(pCustomDataCopy->mpPointer != pCustomDataCopy);
      issea(pCustomDataCopy->mpPointer == pCustomData);

      // Clean up
      success &= pModel->destroyElement(pCustomAnyCopy);
      anyElements = pModel->getElements(dataType);
      issea(anyElements.size() == 2);
      issea(CustomAnyData::mClassCount == 1);

      success &= pModel->destroyElement(pVariantAny);
      anyElements = pModel->getElements(dataType);
      issea(anyElements.size() == 1);
      issea(CustomAnyData::mClassCount == 1);

      success &= pModel->destroyElement(pCustomAny);
      anyElements = pModel->getElements(dataType);
      issea(anyElements.empty() == true);
      issea(CustomAnyData::mClassCount == 0);

      return success;
   }
};

class DeleteParentTestCase : public TestCase
{
public:
   DeleteParentTestCase() :
      TestCase("DeleteParent"),
      mpParentElement(NULL),
      mpChildElement(NULL)
   {
   }

   bool run()
   {
      bool success = true;

      Service<DesktopServices> pDesktop;
      Service<ModelServices> pModel;
      issea(pModel->attach(SIGNAL_NAME(ModelServices, ElementDestroyed),
         Slot(this, &DeleteParentTestCase::elementDeleted)));

      // Create a parent element
      mpParentElement.reset(TestUtilities::getStandardRasterElement());
      issearf(mpParentElement.get() != NULL);

      // Create a child element
      mpChildElement.reset(buildCube("Child Element", FULL_CUBE));
      issearf(mpChildElement.get() != NULL);
      issea(pModel->setElementParent(mpChildElement.get(), mpParentElement.get()));

      // Add a layer to the view for the child element
      string parentName = mpParentElement->getName();

      SpatialDataWindow* pWindow =
         dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(parentName, SPATIAL_DATA_WINDOW));
      issearf(pWindow != NULL);

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issearf(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issearf(pLayerList != NULL);

      AttachmentPtr<Layer> pParentLayer(pLayerList->getLayer(RASTER, mpParentElement.get()));
      issea(pParentLayer.get() != NULL);

      AttachmentPtr<Layer> pChildLayer(pView->createLayer(RASTER, mpChildElement.get()));
      issea(pChildLayer.get() != NULL);

      // Delete the parent element, which deletes the children and the window
      issea(pModel->destroyElement(mpParentElement.get()));

      // Check that all objects were deleted
      issea(mpParentElement.get() == NULL);
      issea(pParentLayer.get() == NULL);
      issea(mpChildElement.get() == NULL);
      issea(pChildLayer.get() == NULL);

      pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(parentName, SPATIAL_DATA_WINDOW));
      issea(pWindow == NULL);

      // Cleanup
      issea(pModel->detach(SIGNAL_NAME(ModelServices, ElementDestroyed),
         Slot(this, &DeleteParentTestCase::elementDeleted)));

      return success;
   }

   void elementDeleted(Subject& subject, const string& signal, const boost::any& value)
   {
      RasterElement* pDeletedElement = dynamic_cast<RasterElement*>(boost::any_cast<DataElement*>(value));
      if ((pDeletedElement != NULL) && (pDeletedElement == mpChildElement.get()))
      {
         // Delete the parent element if the child element was destroyed
         Service<ModelServices> pModel;
         pModel->destroyElement(mpParentElement.get());
      }
   }

private:
   AttachmentPtr<RasterElement> mpParentElement;
   AttachmentPtr<RasterElement> mpChildElement;
};

class DeleteChildTestCase : public TestCase
{
public:
   DeleteChildTestCase() :
      TestCase("DeleteChild"),
      mpParentElement(NULL),
      mpChildElement(NULL),
      mpSiblingElement(NULL)
   {
   }

   bool run()
   {
      bool success = true;

      Service<DesktopServices> pDesktop;
      Service<ModelServices> pModel;
      issea(pModel->attach(SIGNAL_NAME(ModelServices, ElementDestroyed),
         Slot(this, &DeleteChildTestCase::elementDeleted)));

      // Create a parent element
      mpParentElement.reset(TestUtilities::getStandardRasterElement());
      issearf(mpParentElement.get() != NULL);

      // Create a child element
      mpChildElement.reset(buildCube("Child Element", FULL_CUBE));
      issearf(mpChildElement.get() != NULL);
      issea(pModel->setElementParent(mpChildElement.get(), mpParentElement.get()));

      // Create another child element
      mpSiblingElement.reset(buildCube("Sibling Element", FULL_CUBE));
      issearf(mpSiblingElement.get() != NULL);
      issea(pModel->setElementParent(mpSiblingElement.get(), mpParentElement.get()));

      // Add a layer to the view for the child elements
      string parentName = mpParentElement->getName();

      SpatialDataWindow* pWindow =
         dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(parentName, SPATIAL_DATA_WINDOW));
      issearf(pWindow != NULL);

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issearf(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issearf(pLayerList != NULL);

      AttachmentPtr<Layer> pParentLayer(pLayerList->getLayer(RASTER, mpParentElement.get()));
      issea(pParentLayer.get() != NULL);

      AttachmentPtr<Layer> pChildLayer(pView->createLayer(RASTER, mpChildElement.get()));
      issea(pChildLayer.get() != NULL);

      AttachmentPtr<Layer> pSiblingLayer(pView->createLayer(RASTER, mpSiblingElement.get()));
      issea(pSiblingLayer.get() != NULL);

      // Delete the parent element, which deletes the children and the window
      issea(pModel->destroyElement(mpParentElement.get()));

      // Check that all objects were deleted
      issea(mpParentElement.get() == NULL);
      issea(pParentLayer.get() == NULL);
      issea(mpChildElement.get() == NULL);
      issea(pChildLayer.get() == NULL);
      issea(mpSiblingElement.get() == NULL);
      issea(pSiblingLayer.get() == NULL);

      pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(parentName, SPATIAL_DATA_WINDOW));
      issea(pWindow == NULL);

      // Cleanup
      issea(pModel->detach(SIGNAL_NAME(ModelServices, ElementDestroyed),
         Slot(this, &DeleteChildTestCase::elementDeleted)));

      return success;
   }

   void elementDeleted(Subject& subject, const string& signal, const boost::any& value)
   {
      RasterElement* pDeletedElement = dynamic_cast<RasterElement*>(boost::any_cast<DataElement*>(value));
      if ((pDeletedElement != NULL) && (pDeletedElement == mpChildElement.get()))
      {
         // Ensure that the child element was deleted before the sibling element
         tst_assert(mpSiblingElement.get() != NULL);

         // Delete the sibling element if the child element was destroyed
         Service<ModelServices> pModel;
         pModel->destroyElement(mpSiblingElement.get());
      }
   }

private:
   AttachmentPtr<RasterElement> mpParentElement;
   AttachmentPtr<RasterElement> mpChildElement;
   AttachmentPtr<RasterElement> mpSiblingElement;
};

class ModelTestSuite : public TestSuiteNewSession
{
public:
   ModelTestSuite() : TestSuiteNewSession("Model")
   {
      addTestCase(new SessionTestCase);
      addTestCase(new RenameAndReparentDataElement);
      addTestCase(new BitMaskTestCase);
      addTestCase(new BitMaskRegionConstructorTestCase);
      addTestCase(new BandClassStatisticsTestCase);
      addTestCase(new ModelResourceGetTest);
      addTestCase(new DataElementGroupTest);
      addTestCase(new SignatureDataTest);
      addTestCase(new AnyTestCase);
      addTestCase(new DeleteParentTestCase);
      addTestCase(new DeleteChildTestCase);
   }
};

REGISTER_SUITE( ModelTestSuite )
