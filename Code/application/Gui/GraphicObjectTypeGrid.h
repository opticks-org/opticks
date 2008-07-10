/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICOBJECTTYPEGRID_H
#define GRAPHICOBJECTTYPEGRID_H

#include "PixmapGrid.h"

#include "EnumWrapper.h"
#include "PixmapGridButton.h"
#include "TypesFile.h"

#include <QtGui/QPixmap>

#include <map>

class GraphicObjectTypeGrid : public PixmapGrid
{
   Q_OBJECT

public:
   enum ModeEnum { VIEW_ANNOTATION, PRODUCT_ANNOTATION, PLOT_ANNOTATION, VIEW_AOI };
   typedef EnumWrapper<ModeEnum> Mode;

   GraphicObjectTypeGrid(Mode modeValue, QWidget* pParent);

   void setMode(Mode newMode);
   Mode getMode() const;
   void setCurrentValue(GraphicObjectType value);
   GraphicObjectType getCurrentValue() const;

signals: 
   void valueChanged(GraphicObjectType value);

private slots:
   void translateChange(const QString&);

private:
   Mode mMode;
   std::map<GraphicObjectType, QPixmap> mPixmaps;
};

class GraphicObjectTypeButton : public PixmapGridButton
{
   Q_OBJECT

public:
   GraphicObjectTypeButton(GraphicObjectTypeGrid::Mode modeValue, QWidget* pParent);

   GraphicObjectTypeGrid::Mode getMode() const;
   void setMode(GraphicObjectTypeGrid::Mode newMode);
   void setCurrentValue(GraphicObjectType value);
   GraphicObjectType getCurrentValue() const;

signals:
   void valueChanged(GraphicObjectType value);
};

#endif
