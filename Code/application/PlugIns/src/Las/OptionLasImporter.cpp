/*
 * The information in this file is
 * Copyright(c) 2014 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LasImporter.h"
#include "OptionLasImporter.h"
#include <QtCore/QString>
#include <QtGui/QGridLayout>

OptionLasImporter::OptionLasImporter()
{
    mpWidget = new QWidget( NULL );
    QString title = "LAS Importer Options";
    mpWidget->setWindowTitle( title );
    mpWidget->setMaximumHeight( 110 );

    mpDropDown = new QComboBox( mpWidget );
    mpDropDown->addItem( "Import all points (no thinning)" );
    mpDropDown->addItem( "Set max points to import" );

    mpInputMaxPoints = new QLineEdit( mpWidget );
    mpInputMaxPoints->setValidator( new QIntValidator( 1, 999999999, mpInputMaxPoints ) );
    mpInputMaxPoints->setText( "" );
    mpInputGridSize = new QLineEdit( mpWidget );
    mpInputGridSize->setValidator( new QDoubleValidator( 0.001, 20, 3, mpInputGridSize ) );
    mpInputGridSize->setText( "" );

    mpDropDownLabel = new QLabel( "Select thinning option:" );
    mpMaxPointsLabel = new QLabel( "Enter max number of points to render:" );
    mpGridSizeLabel = new QLabel( "Enter grid size:" );

    QGridLayout* pLayout = new QGridLayout( mpWidget );
    pLayout->setAlignment( pLayout, Qt::AlignTop );
    pLayout->addWidget( mpDropDownLabel, 1, 0 );
    pLayout->addWidget( mpDropDown, 1, 1 );
    pLayout->addWidget( mpMaxPointsLabel, 2, 0 );
    pLayout->addWidget( mpInputMaxPoints, 2, 1 );
    pLayout->addWidget( mpGridSizeLabel, 3, 0 );
    pLayout->addWidget( mpInputGridSize, 3, 1 );

    QObject::connect( mpDropDown, SIGNAL( currentIndexChanged( int ) ), this, SLOT( indexChangedEvent( int ) ) );
    mpInputMaxPoints->setDisabled( true );
    mpInputGridSize->setDisabled( true );
    mpMaxPointsLabel->setDisabled( true );
    mpGridSizeLabel->setDisabled( true );
}

OptionLasImporter::~OptionLasImporter()
{
    delete mpDropDownLabel;
    delete mpMaxPointsLabel;
    delete mpGridSizeLabel;
    delete mpDropDown;
    delete mpInputMaxPoints;
    delete mpWidget;
}

QWidget* OptionLasImporter::getWidget()
{
    return mpWidget;
}

QComboBox* OptionLasImporter::getDropDown()
{
    return mpDropDown;
}

QLineEdit* OptionLasImporter::getInputMaxPoints()
{
    return mpInputMaxPoints;
}

QLineEdit* OptionLasImporter::getInputGridSize()
{
    return mpInputGridSize;
}

void OptionLasImporter::indexChangedEvent( int newIndex )
{
    switch ( newIndex )
    {
    case LasImporter::THIN_NONE:
        mpInputMaxPoints->setDisabled( true );
        mpInputGridSize->setDisabled( true );
        mpMaxPointsLabel->setDisabled( true );
        mpGridSizeLabel->setDisabled( true );
        break;
    case LasImporter::THIN_MAX_POINTS:
        mpInputMaxPoints->setEnabled( true );
        mpInputGridSize->setDisabled( true );
        mpMaxPointsLabel->setEnabled( true );
        mpGridSizeLabel->setDisabled( true );
        break;
    default:
        break;
    }
}