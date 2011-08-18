/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GraphicObjectTypeGrid.h"

#include "AppVerify.h"
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
   mPixmaps[ARC_OBJECT] = QPixmap(":/icons/Arc");
   mPixmaps[ARROW_OBJECT] = QPixmap(":/icons/Arrow");
   mPixmaps[EASTARROW_OBJECT] = QPixmap(":/icons/EastArrow");
   mPixmaps[ELLIPSE_OBJECT] = QPixmap(":/icons/Ellipse");
   mPixmaps[FILE_IMAGE_OBJECT] = QPixmap(":/icons/InsertPicture");
   mPixmaps[LATLONINSERT_OBJECT] = QPixmap(":/icons/LatLonInsert");
   mPixmaps[LINE_OBJECT] = QPixmap(":/icons/Line");
   mPixmaps[NORTHARROW_OBJECT] = QPixmap(":/icons/NorthArrow");
   mPixmaps[POLYGON_OBJECT] = QPixmap(":/icons/Polygon");
   mPixmaps[POLYLINE_OBJECT] = QPixmap(":/icons/Polyline");
   mPixmaps[RECTANGLE_OBJECT] = QPixmap(":/icons/Rectangle");
   mPixmaps[ROUNDEDRECTANGLE_OBJECT] = QPixmap(":/icons/RoundedRectangle");
   mPixmaps[SCALEBAR_OBJECT] = QPixmap(":/icons/ScaleBar");
   mPixmaps[TEXT_OBJECT] = QPixmap(":/icons/TextBox");
   mPixmaps[FRAME_LABEL_OBJECT] = QPixmap(":/icons/Clock");
   mPixmaps[MULTIPOINT_OBJECT] = QPixmap(":/icons/Point");
   mPixmaps[HLINE_OBJECT] = QPixmap(":/icons/HLine");
   mPixmaps[VLINE_OBJECT] = QPixmap(":/icons/VLine");
   mPixmaps[ROW_OBJECT] = QPixmap(":/icons/Row");
   mPixmaps[COLUMN_OBJECT] = QPixmap(":/icons/Column");
   mPixmaps[WIDGET_IMAGE_OBJECT] = QPixmap(":/icons/SignatureWindow");
   mPixmaps[VIEW_OBJECT] = QPixmap(":/icons/InsertGLView");
   mPixmaps[TRIANGLE_OBJECT] = QPixmap(":/icons/Triangle");

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

   // Connect to the button signal instead of the grid signal to ensure the member
   // signal is emitted when the user selects a pixmap in the button menu
   VERIFYNR(connect(this, SIGNAL(valueChanged(const QString&)), this, SLOT(translateChange(const QString&))));
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

void GraphicObjectTypeButton::translateChange(const QString& identifier)
{
   GraphicObjectType objectType = StringUtilities::fromXmlString<GraphicObjectType>(identifier.toStdString());
   emit valueChanged(objectType);
}
