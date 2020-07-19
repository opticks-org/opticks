/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayer.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppConfig.h"
#include "ApplicationServicesImp.h"
#include "assert.h"
#include "Classification.h"
#include "ClassificationImp.h"
#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "GraphicGroup.h"
#include "GraphicObject.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ObjectResource.h"
#include "UtilityServicesImp.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#include <QtCore/QString>
#include <string>

using namespace std;


class ClassificationParametersTest : public TestCase
{
public:
   ClassificationParametersTest() : TestCase("Classification") {}

   bool run()
   {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Re-implement when Session Save/Load is done (tjohnson)")
#if 0
      bool success = true;
      RasterElement *pRasterElement = NULL;
      SpatialDataWindow *pWindow = NULL;
      SpatialDataView *pView = NULL;

      ProjectManager *pProjManager = NULL;
      pProjManager = ApplicationServicesImp::instance()->getProjectManager();
      issea( pProjManager != NULL );

      Project *pProject = NULL;
      pProject = pProjManager->createProject( "ClassificationProject" );
      issea( pProject != NULL );

      string testDataPath = TestUtilities::getTestDataPath();
      string filename = testDataPath + "daytonchip.sio";
      string tempDataPath;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempDataPath = pTempPath->getFullPathAndName();
      }
      string tempOutputFile = tempDataPath + "/daytonchip.sio";

      QString temp = tempOutputFile.c_str();
      temp.replace( "\\", "/" );

      tempOutputFile = temp.toStdString();

      // I'll need to import a cube, and then export it to the Opticks temp directory.
      // Because the AOIs and Annotations, etc. are saved into the same directory as the
      // cube when saving a project file, I'll need a directory that has write permission.
      pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      issea( pWindow != NULL );
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      RasterDataDescriptor *pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDescriptor != NULL );

      issea( exportDataSet( pRasterElement, tempOutputFile, pDescriptor, "Standard Format Exporter" ) == true );

      // now close the original cube and open the copy
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      pRasterElement = NULL;
      pWindow = NULL;
      pWindow = TestUtilities::loadDataSet( tempOutputFile, "SIO Importer" );
      issea( pWindow != NULL );

      string sWindowName = pWindow->getName();
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sWindowName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterDataDescriptor *pDescriptorSensor = dynamic_cast<RasterDataDescriptor*>(
         pRasterElement->getDataDescriptor() );
      issea( pDescriptorSensor != NULL );

      // create an AOI layer
      DataDescriptor *pDescriptorAoi = NULL;
      pDescriptorAoi = ModelServicesImp::instance()->createDataDescriptor( "AoiName", "AoiElement", pRasterElement );
      issea( pDescriptorAoi != NULL );
      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>( ModelServicesImp::instance()->createElement( pDescriptorAoi ) );
      issea( pAoi != NULL );

      AoiLayer* pAOILayer = NULL;
      pAOILayer = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi, "AoiName" ) );
      issea( pAOILayer != NULL );

      GraphicObject *pRect = pAoi->getGroup()->addObject( RECTANGLE_OBJECT );
      issea( pRect != NULL && pRect->getGraphicObjectType() == RECTANGLE_OBJECT );
      pRect->setBoundingBox( LocationType( 390, 190 ), LocationType( 590, 370 ) );

      // create an annotation layer
      AnnotationLayer *pAnnotationLayer = NULL;
      pAnnotationLayer = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, NULL, "AnnotationTest" ) );
      issea( pAnnotationLayer != NULL );

      GraphicObject *pEllipse = NULL;
      pEllipse = static_cast<GraphicObject*>( pAnnotationLayer->addObject( ROUNDEDRECTANGLE_OBJECT ) );
      issea( pEllipse != NULL );

      pEllipse->setBoundingBox( LocationType( 140, 360 ), LocationType( 380, 540 ) );
      pEllipse->setFillColor( ColorType( 0, 0, 255 ) );
      pEllipse->setFillStyle( HATCH );
      pEllipse->setHatchStyle( BOXED_CROSS_HAIR );

      // create a GCP layer
      DataDescriptor *pDescriptorGcp = NULL;
      pDescriptorGcp = ModelServicesImp::instance()->createDataDescriptor( "TestList", "GcpList", pRasterElement );
      issea( pDescriptorGcp != NULL );
      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( pDescriptorGcp ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      GcpPoint point1, point2, point3, point4, point5;
      point1.mPixel.mX = 110;
      point1.mPixel.mY = 115;
      point1.mCoordinate.mX = 1.1;
      point1.mCoordinate.mY = 1.2;
      gcpListPoints.push_back( point1 );

      point2.mPixel.mX = 120;
      point2.mPixel.mY = 125;
      point2.mCoordinate.mX = 2.1;
      point2.mCoordinate.mY = 2.2;
      gcpListPoints.push_back( point2 );

      point3.mPixel.mX = 130;
      point3.mPixel.mY = 135;
      point3.mCoordinate.mX = 3.1;
      point3.mCoordinate.mY = 3.2;
      gcpListPoints.push_back( point3 );

      point4.mPixel.mX = 140;
      point4.mPixel.mY = 145;
      point4.mCoordinate.mX = 4.1;
      point4.mCoordinate.mY = 4.2;
      gcpListPoints.push_back( point4 );

      point5.mPixel.mX = 150;
      point5.mPixel.mY = 155;
      point5.mCoordinate.mX = 5.1;
      point5.mCoordinate.mY = 5.2;
      gcpListPoints.push_back( point5 );
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      // set the classifications of the elements
      Classification *pAoiClassification = NULL;
      Classification *pGcpClassification = NULL;
      Classification *pCubeClassification = NULL;

      pAoiClassification = static_cast<Classification*>( Service<ObjectFactory>()->createObject( "Classification" ) );
      issea( pAoiClassification != NULL );
      pGcpClassification = static_cast<Classification*>( Service<ObjectFactory>()->createObject( "Classification" ) );
      issea( pGcpClassification != NULL );
      pCubeClassification = static_cast<Classification*>( Service<ObjectFactory>()->createObject( "Classification" ) );
      issea( pCubeClassification != NULL );

      DateTime *pDeClassDate = NULL;
      DateTime *pDowngradeDate = NULL;
      DateTime *pSecuritySrcDate = NULL;

      pDeClassDate = static_cast<DateTime*>( Service<ObjectFactory>()->createObject( "DateTime" ) );
      issea( pDeClassDate != NULL );
      pDowngradeDate = static_cast<DateTime*>( Service<ObjectFactory>()->createObject( "DateTime" ) );
      issea( pDowngradeDate != NULL );
      pSecuritySrcDate = static_cast<DateTime*>( Service<ObjectFactory>()->createObject( "DateTime" ) );
      issea( pSecuritySrcDate != NULL );

      issea( pDeClassDate->set( 2000, 11, 17, 9, 59, 01 ) == true );
      issea( pDeClassDate->isValid() == true );
      issea( pDowngradeDate->set( 2012, 2, 29, 12, 13, 14 ) == true );
      issea( pDowngradeDate->isValid() == true );
      issea( pSecuritySrcDate->set( 1979, 6, 7, 8, 58, 59 ) == true );
      issea( pSecuritySrcDate->isValid() == true );

      pAoiClassification->setAuthority( "Authority" );
      pAoiClassification->setAuthorityType( "AuthorityType" );
      pAoiClassification->setCodewords( "Codewords are Codewords" );
      pAoiClassification->setCountryCode( "Country Code" );
      pAoiClassification->setDeclassificationDate( pDeClassDate );
      pAoiClassification->setDeclassificationExemption( "Exemption" );
      pAoiClassification->setDescription( "Description" );
      pAoiClassification->setDowngradeDate( pDowngradeDate );
      pAoiClassification->setFileControl( "FileControl" );
      pAoiClassification->setFileCopyNumber( "CopyNumber" );
      pAoiClassification->setFileDowngrade( "FileDowngrade" );
      pAoiClassification->setFileNumberOfCopies( "NumberOfCopies" );
      pAoiClassification->setFileReleasing( "FileReleasing" );
      pAoiClassification->setLevel( "NotARealClassification" );
      pAoiClassification->setSecurityControlNumber( "SecurityControl" );
      pAoiClassification->setSecuritySourceDate( pSecuritySrcDate );
      pAoiClassification->setSystem( "SomeSystem" );

      pGcpClassification->setAuthority( "Authority2" );
      pGcpClassification->setAuthorityType( "AuthorityType2" );
      pGcpClassification->setCodewords( "Codewords are Codewords2" );
      pGcpClassification->setCountryCode( "Country Code2" );
      pGcpClassification->setDeclassificationDate( pDeClassDate );
      pGcpClassification->setDeclassificationExemption( "Exemption2" );
      pGcpClassification->setDescription( "Description2" );
      pGcpClassification->setDowngradeDate( pDowngradeDate );
      pGcpClassification->setFileControl( "FileControl2" );
      pGcpClassification->setFileCopyNumber( "CopyNumber2" );
      pGcpClassification->setFileDowngrade( "FileDowngrade2" );
      pGcpClassification->setFileNumberOfCopies( "NumberOfCopies2" );
      pGcpClassification->setFileReleasing( "FileReleasing2" );
      pGcpClassification->setLevel( "NotARealClassification2" );
      pGcpClassification->setSecurityControlNumber( "SecurityControl2" );
      pGcpClassification->setSecuritySourceDate( pSecuritySrcDate );
      pGcpClassification->setSystem( "SomeSystem2" );

      pCubeClassification->setAuthority( "Authority3" );
      pCubeClassification->setAuthorityType( "AuthorityType3" );
      pCubeClassification->setCodewords( "Codewords are Codewords3" );
      pCubeClassification->setCountryCode( "Country Code3" );
      pCubeClassification->setDeclassificationDate( pDeClassDate );
      pCubeClassification->setDeclassificationExemption( "Exemption3" );
      pCubeClassification->setDescription( "Description3" );
      pCubeClassification->setDowngradeDate( pDowngradeDate );
      pCubeClassification->setFileControl( "FileControl3" );
      pCubeClassification->setFileCopyNumber( "CopyNumber3" );
      pCubeClassification->setFileDowngrade( "FileDowngrade3" );
      pCubeClassification->setFileNumberOfCopies( "NumberOfCopies3" );
      pCubeClassification->setFileReleasing( "FileReleasing3" );
      pCubeClassification->setLevel( "NotARealClassification3" );
      pCubeClassification->setSecurityControlNumber( "SecurityControl3" );
      pCubeClassification->setSecuritySourceDate( pSecuritySrcDate );
      pCubeClassification->setSystem( "SomeSystem3" );

      pDescriptorSensor->setClassification( pCubeClassification );
      pDescriptorAoi->setClassification( pAoiClassification );
      pDescriptorGcp->setClassification( pGcpClassification );

      // export the cube so the Classification info will be saved
      issea( exportDataSet( pRasterElement, tempOutputFile, pDescriptorSensor, "Standard Format Exporter" ) );

      // add the cube to the project
      pProject->addCube( pRasterElement );

      pProject->setFilename( tempDataPath + "/TestProject.cpf" );
      issea( pProject->save() );

      issea( pProjManager->closeProject( pProject ) );
      pWindow = NULL;
      pView = NULL;
      pProject = NULL;

      // now open the saved project
      pProject = pProjManager->openProject( ( tempDataPath + "/TestProject.cpf" ).c_str() );
      issea( pProject != NULL );

      pWindow = dynamic_cast<SpatialDataWindow*>(Service<DesktopServices>()->getWindow(sWindowName,
         SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      // get the SensorData
      RasterElement *pProjectRasterElement = NULL;
      pProjectRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( sWindowName, "RasterElement", NULL ) );
      issea( pProjectRasterElement != NULL );

      // get the AOI
      AoiElement *pProjectAoi = NULL;
      pProjectAoi = static_cast<AoiElement*>(
         ModelServicesImp::instance()->getElement( "AoiName", "AoiElement", pProjectRasterElement ) );
      issea( pProjectAoi != NULL );

      // get the GCP list
      GcpList* pProjectGcpList = NULL;
      pProjectGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->getElement( "TestList", "GcpList", pProjectRasterElement ) );
      issea( pProjectGcpList != NULL );

      const Classification *pProjectCubeClass = NULL;
      pProjectCubeClass = pProjectRasterElement->getClassification();
      issea( pProjectCubeClass != NULL );

      const Classification *pProjectAoiClass = NULL;
      pProjectAoiClass = pProjectAoi->getClassification();
      issea( pProjectAoiClass != NULL );

      const Classification *pProjectGcpListClass = NULL;
      pProjectGcpListClass = pProjectGcpList->getClassification();
      issea( pProjectGcpListClass != NULL );

      issea( pProjectAoiClass->getAuthority() == pAoiClassification->getAuthority() );
      issea( pProjectAoiClass->getAuthorityType() == pAoiClassification->getAuthorityType() );
      issea( pProjectAoiClass->getCodewords() == pAoiClassification->getCodewords() );
      issea( pProjectAoiClass->getCountryCode() == pAoiClassification->getCountryCode() );
      issea( pProjectAoiClass->getDeclassificationDate()->getStructured() == pAoiClassification->getDeclassificationDate()->getStructured() );
      issea( pProjectAoiClass->getDeclassificationExemption() == pAoiClassification->getDeclassificationExemption() );
      issea( pProjectAoiClass->getDescription() == pAoiClassification->getDescription() );
      issea( pProjectAoiClass->getDowngradeDate()->getStructured() == pAoiClassification->getDowngradeDate()->getStructured() );
      issea( pProjectAoiClass->getFileControl() == pAoiClassification->getFileControl() );
      issea( pProjectAoiClass->getFileCopyNumber() == pAoiClassification->getFileCopyNumber() );
      issea( pProjectAoiClass->getFileDowngrade() == pAoiClassification->getFileDowngrade() );
      issea( pProjectAoiClass->getFileNumberOfCopies() == pAoiClassification->getFileNumberOfCopies() );
      issea( pProjectAoiClass->getFileReleasing() == pAoiClassification->getFileReleasing() );
      issea( pProjectAoiClass->getLevel() == pAoiClassification->getLevel() );
      issea( pProjectAoiClass->getSecurityControlNumber() == pAoiClassification->getSecurityControlNumber() );
      issea( pProjectAoiClass->getSecuritySourceDate()->getStructured() == pAoiClassification->getSecuritySourceDate()->getStructured() );
      issea( pProjectAoiClass->getSystem() == pAoiClassification->getSystem() );

      issea( pProjectGcpListClass->getAuthority() == pGcpClassification->getAuthority() );
      issea( pProjectGcpListClass->getAuthorityType() == pGcpClassification->getAuthorityType() );
      issea( pProjectGcpListClass->getCodewords() == pGcpClassification->getCodewords() );
      issea( pProjectGcpListClass->getCountryCode() == pGcpClassification->getCountryCode() );
      issea( pProjectGcpListClass->getDeclassificationDate()->getStructured() == pGcpClassification->getDeclassificationDate()->getStructured() );
      issea( pProjectGcpListClass->getDeclassificationExemption() == pGcpClassification->getDeclassificationExemption() );
      issea( pProjectGcpListClass->getDescription() == pGcpClassification->getDescription() );
      issea( pProjectGcpListClass->getDowngradeDate()->getStructured() == pGcpClassification->getDowngradeDate()->getStructured() );
      issea( pProjectGcpListClass->getFileControl() == pGcpClassification->getFileControl() );
      issea( pProjectGcpListClass->getFileCopyNumber() == pGcpClassification->getFileCopyNumber() );
      issea( pProjectGcpListClass->getFileDowngrade() == pGcpClassification->getFileDowngrade() );
      issea( pProjectGcpListClass->getFileNumberOfCopies() == pGcpClassification->getFileNumberOfCopies() );
      issea( pProjectGcpListClass->getFileReleasing() == pGcpClassification->getFileReleasing() );
      issea( pProjectGcpListClass->getLevel() == pGcpClassification->getLevel() );
      issea( pProjectGcpListClass->getSecurityControlNumber() == pGcpClassification->getSecurityControlNumber() );
      issea( pProjectGcpListClass->getSecuritySourceDate()->getStructured() == pGcpClassification->getSecuritySourceDate()->getStructured() );
      issea( pProjectGcpListClass->getSystem() == pGcpClassification->getSystem() );

      issea( pProjectCubeClass->getAuthority() == pCubeClassification->getAuthority() );
      issea( pProjectCubeClass->getAuthorityType() == pCubeClassification->getAuthorityType() );
      issea( pProjectCubeClass->getCodewords() == pCubeClassification->getCodewords() );
      issea( pProjectCubeClass->getCountryCode() == pCubeClassification->getCountryCode() );
      issea( pProjectCubeClass->getDeclassificationDate()->getStructured() == pCubeClassification->getDeclassificationDate()->getStructured() );
      issea( pProjectCubeClass->getDeclassificationExemption() == pCubeClassification->getDeclassificationExemption() );
      issea( pProjectCubeClass->getDescription() == pCubeClassification->getDescription() );
      issea( pProjectCubeClass->getDowngradeDate()->getStructured() == pCubeClassification->getDowngradeDate()->getStructured() );
      issea( pProjectCubeClass->getFileControl() == pCubeClassification->getFileControl() );
      issea( pProjectCubeClass->getFileCopyNumber() == pCubeClassification->getFileCopyNumber() );
      issea( pProjectCubeClass->getFileDowngrade() == pCubeClassification->getFileDowngrade() );
      issea( pProjectCubeClass->getFileNumberOfCopies() == pCubeClassification->getFileNumberOfCopies() );
      issea( pProjectCubeClass->getFileReleasing() == pCubeClassification->getFileReleasing() );
      issea( pProjectCubeClass->getLevel() == pCubeClassification->getLevel() );
      issea( pProjectCubeClass->getSecurityControlNumber() == pCubeClassification->getSecurityControlNumber() );
      issea( pProjectCubeClass->getSecuritySourceDate()->getStructured() == pCubeClassification->getSecuritySourceDate()->getStructured() );
      issea( pProjectCubeClass->getSystem() == pCubeClassification->getSystem() );

      issea( pProjManager->closeProject( pProject ) );
      ModelServicesImp::instance()->destroyDataDescriptor( pDescriptorAoi );
      ModelServicesImp::instance()->destroyDataDescriptor( pDescriptorGcp );
      pDescriptorAoi = NULL;
      pDescriptorGcp = NULL;

      return success;
#endif
      return false;
   }
};


class ClassificationValidityTest : public TestCase
{
public:
   ClassificationValidityTest() : TestCase("ClassificationValidityTest") {}

   bool run()
   {
      bool success = true;

      FactoryResource<Classification> pClass1;
      string errorMessage;
      pClass1->setLevel( "T" );
      issea( dynamic_cast<ClassificationImp*>( pClass1.get() )->isValid( errorMessage ) == false );
      pClass1->setCodewords( "test1 test2" );
      issea( dynamic_cast<ClassificationImp*>( pClass1.get() )->isValid( errorMessage ) == true );

      FactoryResource<Classification> pClass2;
      pClass2->setLevel( "U" );
      issea( dynamic_cast<ClassificationImp*>( pClass2.get() )->isValid( errorMessage ) == true );

      return success;
   }
};

class ClassificationTextTest : public TestCase
{
public:
   ClassificationTextTest() :
      TestCase("ClassText")
   {}

   bool run()
   {
      bool success = true;

      FactoryResource<Classification> pClass1;
      string classText;

      UtilityServicesImp* pUtilImp = UtilityServicesImp::instance();
      issearf(pUtilImp != NULL);

      const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
      issearf(pSupportFilesPath != NULL);

      string supportFilesPath = pSupportFilesPath->getFullPathAndName();
      issea(pUtilImp->loadSecurityMarkings(supportFilesPath + SLASH + "SecurityMarkings") == true);

      pUtilImp->overrideDefaultClassification("UNCLASSIFIED");

      pClass1->setLevel("TOP SECRET");
      pClass1->setCodewords("CW1 CW2");
      pClass1->setFileReleasing("ORCON REL\\ TO DSEN");
      pClass1->setCountryCode("CC1 CC2");
      pClass1->setSystem("System1");
      pClass1->setDeclassificationExemption("EX21");
      pClass1->getClassificationText(classText);
      issea(classText == "TOP SECRET//CW1/CW2//System1//ORCON/REL TO USA, CC1, CC2/DSEN//EX21");
      pClass1->setLevel("UNCLASSIFIED");
      pClass1->getClassificationText(classText);
      issea(classText == "UNCLASSIFIED");

      pClass1->setLevel("TOP SECRET");
      pClass1->setCodewords("SI TK");
      pClass1->setCountryCode("USA");
      pClass1->setFileReleasing("ORCON REL\\ TO NOFORN DSEN");
      pClass1->setDeclassificationExemption("MR");
      pClass1->getClassificationText(classText);
      issea(classText == "TOP SECRET//SI/TK//ORCON/NOFORN/DSEN//MR");

      FactoryResource<Classification> pClass2;
      string classText2;

      issea(pClass2->setClassification(classText) == true);
      pClass2->getClassificationText(classText2);
      issea(classText2 == classText);

      FactoryResource<DateTime> pDeclassDate;
      pDeclassDate->set(2001, 1, 1);
      pClass1->setDeclassificationDate(pDeclassDate.get());
      pClass1->getClassificationText(classText);
      issea(classText == "TOP SECRET//SI/TK//ORCON/NOFORN/DSEN//MR");
      pClass1->setDeclassificationExemption(string());
      pClass1->getClassificationText(classText);
      issea(classText == "TOP SECRET//SI/TK//ORCON/NOFORN/DSEN//20010101");

      issea(pClass2->setClassification(classText) == true);
      pClass2->getClassificationText(classText2);
      issea(classText2 == classText);

      pClass1->setFileReleasing("ORCON REL\\ TO DSEN");
      pClass1->getClassificationText(classText);
      issea(classText == "TOP SECRET//SI/TK//ORCON/DSEN//20010101");
      pClass1->setCountryCode("CC1 CC2");
      pClass1->getClassificationText(classText);
      issea(classText == "TOP SECRET//SI/TK//ORCON/REL TO USA, CC1, CC2/DSEN//20010101");

      pUtilImp->overrideDefaultClassification("TOP SECRET//SI/TK");

      FactoryResource<Classification> pClass3;
      pClass3->getClassificationText(classText);
      issea(classText == "TOP SECRET//SI/TK");
      issea(pClass3->getCodewords() == "SI TK");

      pClass3->setLevel("U");
      pClass3->getClassificationText(classText);
      issea(classText == "UNCLASSIFIED");

      pClass3->setLevel(string());
      pClass3->setCodewords(string());
      pClass3->getClassificationText(classText);
      issea(classText == "TOP SECRET");
      pClass3->setCodewords("SI TK");
      pClass3->getClassificationText(classText);
      issea(classText == "TOP SECRET//SI/TK");
      pClass3->setLevel("U");
      pClass3->getClassificationText(classText);
      issea(classText == "UNCLASSIFIED");

      pUtilImp->overrideDefaultClassification(string());
      return success;
   }
};

class ClassificationTestSuite : public TestSuiteNewSession
{
public:
   ClassificationTestSuite() : TestSuiteNewSession( "Classification" )
   {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Re-add when Session Save/Load is done (tjohnson)")
      //addTestCase( new ClassificationParametersTest );
      addTestCase( new ClassificationTextTest );
      addTestCase( new ClassificationValidityTest );
   }
};

REGISTER_SUITE( ClassificationTestSuite )
