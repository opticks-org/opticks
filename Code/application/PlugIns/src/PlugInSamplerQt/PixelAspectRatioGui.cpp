/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServices.h"
#include "PixelAspectRatioGui.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "Service.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <QtCore/QFileInfo>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

PixelAspectRatioGui::PixelAspectRatioGui( QWidget* pParent, const char* pName, bool modal )
: QDialog(pParent)
{
   if (pName == NULL)
   {
      setObjectName( "PixelAspectRatioGui" );
   }
   setModal( FALSE );

   QGridLayout *pLayout = new QGridLayout( this );

   mpApplyButton = new QPushButton( "Apply Scaling", this );
   mpGenerateViewButton = new QPushButton( "Generate Test View", this );
   mpCubeListCombo = new QComboBox( this );
   mpXScaleFactor = new QDoubleSpinBox( this );
   mpYScaleFactor = new QDoubleSpinBox( this );
   mpCubeListComboLabel = new QLabel( "Cube to Scale", this );
   mpXScaleFactorLabel = new QLabel( "X Scale Factor", this );
   mpYScaleFactorLabel = new QLabel( "Y Scale Factor", this );
   mpCancelButton = new QPushButton( "cancelButton", this );

   pLayout->addWidget( mpApplyButton, 3, 0 );
   pLayout->addWidget( mpGenerateViewButton, 0, 3 );
   pLayout->addWidget( mpCubeListCombo, 0, 0, 1, 2 );
   pLayout->addWidget( mpXScaleFactor, 1, 0 );
   pLayout->addWidget( mpYScaleFactor, 2, 0 );
   pLayout->addWidget( mpCubeListComboLabel, 0, 2 );
   pLayout->addWidget( mpXScaleFactorLabel, 1, 1 );
   pLayout->addWidget( mpYScaleFactorLabel, 2, 1 );
   pLayout->addWidget( mpCancelButton, 3, 3 );

   mpCancelButton->setText("Close");
   resize( QSize(500, 380).expandedTo(minimumSizeHint()) );

   // signals and slots connections
   bool ok = connect( mpCancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
   ok = connect( mpApplyButton, SIGNAL( clicked() ), this, SLOT( applyScale() ) );
   ok = connect( mpGenerateViewButton, SIGNAL( clicked() ), this, SLOT( generateNewView() ) );
   init();
}

/*
*  Destroys the object and frees any allocated resources
*/
PixelAspectRatioGui::~PixelAspectRatioGui()
{
   Service<DesktopServices> pDesktop;
   StepResource pStep( "Pixel Aspect Ratio Closed.", "app", "1E200C4D-003D-4684-B9B0-712D7E17051A" );
   if( mbScalingApplied ) //successful state
   {
      pStep->finalize( Message::Success );
   }
   else //cancelled state
   {
      pStep->finalize( Message::Abort, "Plug-in Cancelled!" );
   }

   SpatialDataWindow* pScaledWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->getWindow(
      "scaledCubeWindow", SPATIAL_DATA_WINDOW ) );
   if( pScaledWindow != NULL )
   {
      pDesktop->deleteWindow( pScaledWindow );
      pScaledWindow = NULL;
   }
}

void PixelAspectRatioGui::init()
{
   Service<ModelServices> pModel;
   mCubeNames = pModel->getElementNames( "RasterElement" );

   for( unsigned int i = 0; i < mCubeNames.size(); i++ )
   {
      mpCubeListCombo->insertItem( i, QString::fromStdString(mCubeNames[i]) );
   }

   mpXScaleFactor->setMinimum( 1 );
   mpYScaleFactor->setMinimum( 1 );
   mpScaledLayer = NULL;

   mpXScaleFactor->setEnabled( false );
   mpYScaleFactor->setEnabled( false );
   mpApplyButton->setEnabled( false );
   mbScalingApplied = false;
}

void PixelAspectRatioGui::applyScale()
{
   Service<DesktopServices> pDesktop;

   SpatialDataWindow* pScaledWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->getWindow( "scaledCubeWindow", SPATIAL_DATA_WINDOW ) );
   if( pScaledWindow != NULL )
   {
      if( mpScaledLayer != NULL )
      {
         mpScaledLayer->setXScaleFactor( mpXScaleFactor->value() );
         mpScaledLayer->setYScaleFactor( mpYScaleFactor->value() );
         mbScalingApplied = true;
      }
   }
}

void PixelAspectRatioGui::generateNewView()
{
   Service<DesktopServices> pDesktop;

   SpatialDataWindow *pWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->getWindow( mCubeNames.at( mpCubeListCombo->currentIndex() ), SPATIAL_DATA_WINDOW ) );
   if( pWindow != NULL )
   {
      SpatialDataView *pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      if( pView != NULL )
      {
         Layer *pLayer = pView->getTopMostLayer( RASTER );
         if( pLayer != NULL )
         {
            SpatialDataWindow* pScaledWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->createWindow( "scaledCubeWindow", SPATIAL_DATA_WINDOW ) );
            if( pScaledWindow != NULL )
            {
               SpatialDataView *pScaledView = dynamic_cast<SpatialDataView*>( pScaledWindow->getView() );
               if( pScaledView != NULL )
               {
                  mpScaledLayer = pLayer->copy( std::string(), true, pLayer->getDataElement() );
                  pScaledView->addLayer( mpScaledLayer );
                  pScaledView->refresh();
                  pScaledView->zoomExtents();

                  mpXScaleFactor->setEnabled( true );
                  mpYScaleFactor->setEnabled( true );
                  mpApplyButton->setEnabled( true );
               }
            }
         }
      }
   }
}

