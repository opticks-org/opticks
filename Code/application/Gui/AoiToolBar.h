/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOITOOLBAR_H
#define AOITOOLBAR_H

#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "ToolBarAdapter.h"

#include "PixmapGrid.h"
#include "PixmapGridButton.h"
#include "TypesFile.h"

class AoiLayer;
class AoiAddModeButton;
class ColorMenu;
class GraphicObjectTypeButton;
class Layer;
class SymbolTypeButton;

/**
 *  Provides means to manage and define areas of interest (AOIs)
 *
 *  The AOI toolbar is divided into three sections.  The first section provides
 *  management capabilities for AOI layers as a whole.  AOIs can be added, deleted,
 *  loaded, saved, made current, and renamed.
 *
 *  The second section of the toolbar provides pixel selection capability for the
 *  AOI.  The user can select one of several selection methods to select, deselect,
 *  or toggle pixels in the AOI.
 *
 *  The third section of the toolbar provides a means to define the display
 *  characteristics of the layer.  The user can set the display marker color and shape.
 *  The user can also invoke
 *  the properties dialog to set additional layer characteristics.
 */
class AoiToolBar : public ToolBarAdapter
{
   Q_OBJECT

public:
   /**
    *  Creates the AOI toolbar.
    *
    *  @param   id
    *           The unique ID for the toolbar.
    *  @param   parent
    *           The widget to which the toolbar is attached.
    */
   AoiToolBar(const std::string& id, QWidget* parent = 0);

   /**
    *  Destroys the AOI toolbar.
    */
   ~AoiToolBar();

   /**
    *  Returns the current AOI layer.
    *
    *  @return  A pointer to the current AOI layer.  NULL is returned if the
    *           toolbar is not associated with a layer.
    */
   Layer* getAoiLayer() const;

   /**
    *  Sets the current pixel selection tool.
    *
    *  This method set the current pixel selection tool type and drawing mode.
    *
    *  @param   eTool
    *           The selection tool type.
    *  @param   eMode
    *           The drawing mode.
    *
    *  @see     AoiToolBar::getSelectionTool
    *  @see     AoiToolBar::getSelectionMode
    */
   void setSelectionTool(GraphicObjectType eTool, ModeType eMode);

   /**
    *  Returns the current pixel selection tool.
    *
    *  @return  The current selection tool type enumerated value.
    *
    *  @see     AoiToolBar::getSelectionMode
    */
   GraphicObjectType getSelectionTool() const;

   /**
    *  Returns the current drawing mode.
    *
    *  @return  The current drawing mode enumerated value.
    *
    *  @see     AoiToolBar::getSelectionTool
    */
   ModeType getSelectionMode() const;

   /**
    *  Returns the current state of the AOI add mode.
    *
    *  @return  The current AOI add mode.
    */
   AoiAddMode getAddMode() const;

   /**
    *  Returns the current state of the AOI show labels flag
    *
    *  @return  The current AOI show labels state.
    */
   bool getAoiShowLabels() const;

   /**
    *  Returns the current state of the AOI show point labels flag
    *
    *  @return  The current AOI show point labels state.
    */
   bool getAoiShowPointLabels() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   bool setAoiLayer(Layer* pLayer);
   void setAddMode(AoiAddMode mode);

signals:
   void graphicObjectTypeChanged(GraphicObjectType type);
   void modeChanged(ModeType mode);

protected:
   void aoiLayerDeleted(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void selectionObjectChanged();
   void mergeAoi();
   void setSelectionTool(GraphicObjectType eTool);
   void setSelectionMode(ModeType eMode);
   void clearAoi();
   void invertAoi();
   void setAoiSymbol(SymbolType markerSymbol);
   void initializeColorMenu();
   void setAoiColor(const QColor& markerColor);
   void changeShowLabelState();
   void setShowPointLabelState(bool showPointLabel);

private:
   AoiToolBar(const AoiToolBar& rhs);
   AoiToolBar& operator=(const AoiToolBar& rhs);

   // Draw mode actions
   QAction* mpDraw;
   QAction* mpErase;
   QAction* mpToggle;
   QAction* mpAoiMoveMode;

   // Display actions
   QAction* mpAoiShowLabels;
   QAction* mpAoiShowPointLabels;

   // AOI add mode
   AoiAddModeButton* mpAddMode;

   // Selection tool actions
   GraphicObjectTypeButton* mpTool;

   // Selection actions
   QAction* mpEraseAll;
   QAction* mpToggleAll;

   // Info actions
   QAction* mpMerge;

   // Symbol actions
   SymbolTypeButton* mpSymbolButton;

   // Color actions
   ColorMenu* mpColorMenu;

   // AOI layer
   AoiLayer* mpAoiLayer;
};

class AoiAddModeGrid : public PixmapGrid
{
   Q_OBJECT

public:
   AoiAddModeGrid(QWidget* pParent);
   void setCurrentValue(AoiAddMode value);
   AoiAddMode getCurrentValue() const;

signals: 
   void valueChanged(AoiAddMode value);

private slots:
   void translateChange(const QString&);

private:
   AoiAddModeGrid(const AoiAddModeGrid& rhs);
   AoiAddModeGrid& operator=(const AoiAddModeGrid& rhs);
};

class AoiAddModeButton : public PixmapGridButton
{
   Q_OBJECT

public:
   AoiAddModeButton(QWidget* pParent);

   void setCurrentValue(AoiAddMode value);
   AoiAddMode getCurrentValue() const;

signals:
   void valueChanged(AoiAddMode value);

private:
   AoiAddModeButton(const AoiAddModeButton& rhs);
   AoiAddModeButton& operator=(const AoiAddModeButton& rhs);
};

#endif
