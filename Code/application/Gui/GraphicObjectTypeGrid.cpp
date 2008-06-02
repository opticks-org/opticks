/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GraphicObjectTypeGrid.h"

#include "AppAssert.h"
#include "AppVerify.h"
#include "Icons.h"
#include "StringUtilities.h"

#include <QtGui/QBitmap>
#include <QtGui/QPainter>

#include <string>
#include <vector>
#include <algorithm>

using namespace std;

GraphicObjectTypeGrid::GraphicObjectTypeGrid(GraphicObjectTypeGrid::Mode modeValue, QWidget* pParent)
: PixmapGrid(pParent)
{
   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   mPixmaps[ARC_OBJECT] = pIcons->mArc;
   mPixmaps[ARROW_OBJECT] = pIcons->mArrow;
   mPixmaps[EASTARROW_OBJECT] = pIcons->mEastArrow;
   mPixmaps[ELLIPSE_OBJECT] = pIcons->mEllipse;
   mPixmaps[FILE_IMAGE_OBJECT] = pIcons->mInsertPict;
   mPixmaps[LATLONINSERT_OBJECT] = pIcons->mLatLonInsert;
   mPixmaps[LINE_OBJECT] = pIcons->mLine;
   mPixmaps[NORTHARROW_OBJECT] = pIcons->mNorthArrow;
   mPixmaps[POLYGON_OBJECT] = pIcons->mPolygon;
   mPixmaps[POLYLINE_OBJECT] = pIcons->mPolyline;
   mPixmaps[RECTANGLE_OBJECT] = pIcons->mRectangle;
   mPixmaps[ROUNDEDRECTANGLE_OBJECT] = pIcons->mRoundedRect;
   mPixmaps[SCALEBAR_OBJECT] = pIcons->mScaleBar;
   mPixmaps[TEXT_OBJECT] = pIcons->mTextBox;
   mPixmaps[FRAME_LABEL_OBJECT] = pIcons->mClock;
   mPixmaps[MULTIPOINT_OBJECT] = pIcons->mPoint;
   mPixmaps[HLINE_OBJECT] = pIcons->mHLine;
   mPixmaps[VLINE_OBJECT] = pIcons->mVLine;
   mPixmaps[ROW_OBJECT] = pIcons->mRow;
   mPixmaps[COLUMN_OBJECT] = pIcons->mColumn;
   mPixmaps[WIDGET_IMAGE_OBJECT] = pIcons->mSignature;
   mPixmaps[VIEW_OBJECT] = pIcons->mInsertGLView;
   mPixmaps[TRIANGLE_OBJECT] = pIcons->mTriangle;

   setMode(modeValue);

   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void GraphicObjectTypeGrid::setMode(GraphicObjectTypeGrid::Mode newMode)
{
   if (!newMode.isValid())
   {
      return;
   }

   if (newMode == mMode)
   {
      return;
   }

   GraphicObjectType orgSelection = getCurrentValue();

   int numRows = 0;
   int numColumns = 0;
   GraphicObjectType defaultObject;
   vector<GraphicObjectType> supportedObjects;
   if (newMode == VIEW_ANNOTATION)
   {
      numRows = 4;
      numColumns = 4;
      defaultObject = RECTANGLE_OBJECT;

      supportedObjects.push_back(RECTANGLE_OBJECT);
      supportedObjects.push_back(ROUNDEDRECTANGLE_OBJECT);
      supportedObjects.push_back(ELLIPSE_OBJECT);
      supportedObjects.push_back(TRIANGLE_OBJECT);

      supportedObjects.push_back(LINE_OBJECT);
      supportedObjects.push_back(POLYLINE_OBJECT);
      supportedObjects.push_back(POLYGON_OBJECT);
      supportedObjects.push_back(ARC_OBJECT);

      supportedObjects.push_back(ARROW_OBJECT);
      supportedObjects.push_back(TEXT_OBJECT);
      supportedObjects.push_back(FRAME_LABEL_OBJECT);
      supportedObjects.push_back(FILE_IMAGE_OBJECT);

      supportedObjects.push_back(NORTHARROW_OBJECT);
      supportedObjects.push_back(EASTARROW_OBJECT);
      supportedObjects.push_back(LATLONINSERT_OBJECT);
      supportedObjects.push_back(SCALEBAR_OBJECT);
   }
   else if (newMode == PLOT_ANNOTATION)
   {
      numRows = 3;
      numColumns = 4;
      defaultObject = RECTANGLE_OBJECT;

      supportedObjects.push_back(RECTANGLE_OBJECT);
      supportedObjects.push_back(ROUNDEDRECTANGLE_OBJECT);
      supportedObjects.push_back(ELLIPSE_OBJECT);
      supportedObjects.push_back(TRIANGLE_OBJECT);

      supportedObjects.push_back(LINE_OBJECT);
      supportedObjects.push_back(POLYLINE_OBJECT);
      supportedObjects.push_back(POLYGON_OBJECT);
      supportedObjects.push_back(ARC_OBJECT);

      supportedObjects.push_back(ARROW_OBJECT);
      supportedObjects.push_back(TEXT_OBJECT);
      supportedObjects.push_back(FRAME_LABEL_OBJECT);
      supportedObjects.push_back(FILE_IMAGE_OBJECT);
   }
   else if (newMode == VIEW_AOI)
   {
      numRows = 3;
      numColumns = 3;
      defaultObject = MULTIPOINT_OBJECT;

      supportedObjects.push_back(MULTIPOINT_OBJECT);
      supportedObjects.push_back(RECTANGLE_OBJECT);
      supportedObjects.push_back(POLYGON_OBJECT);

      supportedObjects.push_back(LINE_OBJECT);
      supportedObjects.push_back(HLINE_OBJECT);
      supportedObjects.push_back(VLINE_OBJECT);

      supportedObjects.push_back(POLYLINE_OBJECT);
      supportedObjects.push_back(ROW_OBJECT);
      supportedObjects.push_back(COLUMN_OBJECT);
   }
   else if (newMode == PRODUCT_ANNOTATION)
   {
      numRows = 4;
      numColumns = 4;
      defaultObject = RECTANGLE_OBJECT;

      supportedObjects.push_back(RECTANGLE_OBJECT);
      supportedObjects.push_back(ROUNDEDRECTANGLE_OBJECT);
      supportedObjects.push_back(ELLIPSE_OBJECT);
      supportedObjects.push_back(TRIANGLE_OBJECT);

      supportedObjects.push_back(LINE_OBJECT);
      supportedObjects.push_back(POLYLINE_OBJECT);
      supportedObjects.push_back(POLYGON_OBJECT);
      supportedObjects.push_back(ARC_OBJECT);

      supportedObjects.push_back(ARROW_OBJECT);
      supportedObjects.push_back(TEXT_OBJECT);
      supportedObjects.push_back(FRAME_LABEL_OBJECT);
      supportedObjects.push_back(FILE_IMAGE_OBJECT);

      supportedObjects.push_back(NORTHARROW_OBJECT);
      supportedObjects.push_back(EASTARROW_OBJECT);
      supportedObjects.push_back(WIDGET_IMAGE_OBJECT);
      supportedObjects.push_back(VIEW_OBJECT);
   }

   //clear any existing pixmaps
   setNumRows(0);
   setNumColumns(0);

   //re-populate the pixmaps
   setNumRows(numRows);
   setNumColumns(numColumns);

   VERIFYNR((numRows * numColumns) >= static_cast<int>(supportedObjects.size()));

   for (vector<GraphicObjectType>::size_type objCount = 0;
        objCount < supportedObjects.size(); ++objCount)
   {
      GraphicObjectType object = supportedObjects[objCount];
      int row = objCount / numColumns;
      int column = objCount % numColumns;
      QPixmap pix = mPixmaps[object];
      setPixmap(row, column, pix,
         QString::fromStdString(StringUtilities::toXmlString(object)),
         QString::fromStdString(StringUtilities::toDisplayString(object)));
   }

   //persist the original selection if possible
   vector<GraphicObjectType>::iterator foundSelection = std::find(supportedObjects.begin(),
      supportedObjects.end(), orgSelection);

   if (foundSelection != supportedObjects.end())
   {
      setCurrentValue(orgSelection);
   }
   else
   {
      setCurrentValue(defaultObject);
   }

   mMode = newMode;
}

GraphicObjectTypeGrid::Mode GraphicObjectTypeGrid::getMode() const
{
   return mMode;
}

void GraphicObjectTypeGrid::setCurrentValue(GraphicObjectType value)
{
   QString strValue = QString::fromStdString(StringUtilities::toXmlString(value));
   setSelectedPixmap(strValue);
}

GraphicObjectType GraphicObjectTypeGrid::getCurrentValue() const
{
   GraphicObjectType retValue;
   string curText = getSelectedPixmapIdentifier().toStdString();
   if (!curText.empty())
   {
      retValue = StringUtilities::fromXmlString<GraphicObjectType>(curText);
   }
   return retValue;
}

void GraphicObjectTypeGrid::translateChange(const QString& strText)
{
   GraphicObjectType curType = StringUtilities::fromXmlString<GraphicObjectType>(strText.toStdString());
   emit valueChanged(curType);
}

GraphicObjectTypeButton::GraphicObjectTypeButton(GraphicObjectTypeGrid::Mode modeValue, QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   GraphicObjectTypeGrid* pGrid = new GraphicObjectTypeGrid(modeValue, this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(pGrid, SIGNAL(valueChanged(GraphicObjectType)), this, SIGNAL(valueChanged(GraphicObjectType))));
}
      
GraphicObjectTypeGrid::Mode GraphicObjectTypeButton::getMode() const
{
   GraphicObjectTypeGrid* pGrid = dynamic_cast<GraphicObjectTypeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      return pGrid->getMode();
   }
   return GraphicObjectTypeGrid::Mode();
}

void GraphicObjectTypeButton::setMode(GraphicObjectTypeGrid::Mode newMode)
{
   GraphicObjectTypeGrid* pGrid = dynamic_cast<GraphicObjectTypeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setMode(newMode);
      refreshMenu();
   }
}

void GraphicObjectTypeButton::setCurrentValue(GraphicObjectType value)
{
   GraphicObjectTypeGrid* pGrid = dynamic_cast<GraphicObjectTypeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(value);
   }
}

GraphicObjectType GraphicObjectTypeButton::getCurrentValue() const
{
   GraphicObjectType retValue;
   GraphicObjectTypeGrid* pGrid = dynamic_cast<GraphicObjectTypeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      retValue = pGrid->getCurrentValue();
   }
   return retValue;
}
