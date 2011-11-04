/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANNOTATIONTOOLBAR_H
#define ANNOTATIONTOOLBAR_H

#include <QtGui/QAction>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFontComboBox>
#include <QtGui/QMenu>
#include <QtGui/QWidgetAction>

#include "ToolBarAdapter.h"
#include "TypesFile.h"

#include <list>

class AnnotationLayerImp;
class ColorMenu;
class FontSizeComboBox;
class GraphicObject;
class GraphicObjectTypeButton;
class Layer;
class PixmapGrid;
class QButtonGroup;

class AnnotationToolBar : public ToolBarAdapter
{
   Q_OBJECT

public:
   AnnotationToolBar(const std::string& id, QWidget* parent = 0);
   ~AnnotationToolBar();

   void optionsModified(Subject &subject, const std::string &signal, const boost::any &v);

   Layer* getAnnotationLayer() const;

public slots:
   bool setAnnotationLayer(Layer* pLayer);
   void setSelectionObject(GraphicObjectType eObject);

signals:
   void graphicObjectTypeChanged(GraphicObjectType type);
   void snapToGridChanged(bool snap);

protected:
   bool eventFilter(QObject* pObject, QEvent* pEvent);
   void annotationLayerDeleted(Subject &subject, const std::string &signal, const boost::any &v);

protected slots:
   void enableSelectionTools();

   void setLayerLocked(bool);
   void initializeTextColorMenu();
   void setTextColor(const QColor& textColor);
   void initializeLineWidthGrid();
   void setLineWidth(const QPixmap& lineWidthPix);
   void initializeLineColorMenu();
   void setLineColor(const QColor& lineColor);
   void initializeFillColorMenu();
   void setFillColor(const QColor& fillColor);
   void setFillStateFalse();
   void setFontFace(const QString& strFont);
   void setFontSize(int fontSize);
   void setFontBold(bool bBold);
   void setFontItalic(bool bItalics);
   void setFontUnderline(bool bUnderline);
   void group();
   void ungroup();
   void popFront();
   void pushBack();
   void nudgeUp();
   void nudgeDown();
   void nudgeLeft();
   void nudgeRight();
   void alignLeft();
   void alignCenter();
   void alignRight();
   void alignTop();
   void alignMiddle();
   void alignBottom();
   void distributeVertically();
   void distributeHorizontally();
   void selectAll();

   void updateDefaultProperties();
   void updateSelectedProperties();
   void updateSelectedProperties(std::list<GraphicObject*>& selectedObjects);
   void modifyLayerProperties();

   void selectionObjectChanged();

   void setSnapToGrid(bool snap);

private:
   AnnotationToolBar(const AnnotationToolBar& rhs);
   AnnotationToolBar& operator=(const AnnotationToolBar& rhs);

   GraphicObjectType getSelectionObject();

   // Lock layer check box
   QCheckBox* mpLayerLocked;

   // Object actions
   QButtonGroup* mpObjectGroup;
   QAction* mpMove;
   QAction* mpRotate;
   GraphicObjectTypeButton* mpObject;

   // Font actions
   QFontComboBox* mpFont_Combo;
   FontSizeComboBox* mpSize_Combo;
   QAction* mpBold;
   QAction* mpItalic;
   QAction* mpUnderline;

   // Text color actions
   ColorMenu* mpTextColorMenu;

   // Line width actions
   QMenu* mpLineWidthMenu;
   PixmapGrid* mpLineWidthGrid;

   // Line color actions
   ColorMenu* mpLineColorMenu;

   // Fill color actions
   ColorMenu* mpFillColorMenu;

   // Snap to grid action
   QAction* mpSnapToGrid;

   // Annotation layer
   AnnotationLayerImp* mpAnnotationLayer;
};

#endif
