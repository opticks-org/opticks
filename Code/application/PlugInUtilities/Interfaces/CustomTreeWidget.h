/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QtCore/QMap>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include "ColorType.h"
#include "EnumWrapper.h"

class CustomEditWidget;
class FileBrowser;

/**
 *  A tree view widget with in-cell editing capabilities.
 *
 *  The CustomTreeWidget extends the QTreeWidget class to provide in-cell
 *  editing to allow the user to change the value of a particular cell.  The
 *  following edit capabilities are provided:
 *
 *  <pre>
 *  Edit Capability     Description
 *  ==================  =======================================================
 *  None                The cell cannot be edited by the user.
 *  Check Box           The cell contains a check box that the user can toggle
 *                      on and off.  %Text can optionally be displayed next to
 *                      the check box.
 *  Color               The cell contains a colored pixmap indicating the
 *                      selected color for the cell.  %Text can optionally be
 *                      displayed next to the colored pixmap.
 *  Line Edit           A single-line edit is invoked for the user to enter
 *                      text when the user activates the cell.
 *  Custom Line Edit    A single-line edit is invoked for the user to enter
 *                      text when the user activates the cell.  Enhancements
 *                      or restrictions may be placed on the line edit such as
 *                      a custom alignment or echo mode, or a validator or
 *                      completer by creating a QLineEdit independently from
 *                      the tree widget and calling the setCustomLineEdit()
 *                      method.
 *  File Browser        A single-line edit box with a browse button is invoked
 *                      when the user activates the cell.  When the user clicks
 *                      the browse button, a file selection dialog is invoked
 *                      for the user to select a single file.
 *  Directory Browser   A single-line edit box with a browse button is invoked
 *                      when the user activates the cell.  When the user clicks
 *                      the browse button, a file selection dialog is invoked
 *                      for the user to select a directory.
 *  Shortcut Edit       A single-line edit box is invoked when the user
 *                      activates the cell.  As the user types, the keystrokes
 *                      are converted into key combinations that displayed in
 *                      the edit box.  Examples of key combinations include
 *                      "Ctrl+S", and "Shift+F5".
 *  Combo Box           A drop-down combo box is invoked when the user
 *                      activates the cell.  The combo box items are populated
 *                      by creating a QComboBox indepentently from the tree
 *                      widget and calling the setComboBox() method.
 *  Spin Box            A spin box is invoked when the user activates the cell.
 *                      The step size and range of values in the spin box are
 *                      set by creating a QSpinBox independently from the tree
 *                      widget and calling the setSpinBox() method.
 *  Double Spin Box     A double precision floating-point spin box is invoked when
 *                      the user activates the cell.  The step size and range of values
 *                      in the spin box are set by creating a QDoubleSpinBox independently
 *                      from the tree widget and calling the setDoubleSpinBox() method.
 *  Custom Widget       A custom edit widget is invoked when the user activates the cell.
 *                      The widget must be derived from CustomEditWidget and provide implementations
 *                      for the pure virtual methods.  The widget must be created independently
 *                      from the tree widget and set into the tree widget by calling setCustomEditWidget().
 *  </pre>
 *
 *  The default row height of tree widget items that are added to a custom tree
 *  widget is 20 pixels to provide additional room for an edit widget.
 *
 *  Each item can be drawn with horizontal gridlines, vertical gridlines, or
 *  both.
 */
class CustomTreeWidget : public QTreeWidget
{
   Q_OBJECT

public:
   /**
    *  Creates a new, empty custom tree widget.
    *
    *  @param   pParent
    *           The parent widget.
    */
   CustomTreeWidget(QWidget* pParent = 0);

   /**
    *  Destroys the custom tree widget and all child tree widget items.
    */
   virtual ~CustomTreeWidget();

   /**
    *  Specifies the edit widget used in a cell of the tree widget.
    *
    *  @see        setCellWidgetType()
    */
   enum WidgetTypeEnum
   {
      NO_WIDGET = 0,     /**< A read-only cell that contains no edit widget. */
      LINE_EDIT,         /**< A single-line edit box is invoked when the user
                              activates the cell.\  The line edit is
                              automatically created and managed by the custom
                              tree widget. */
      CUSTOM_LINE_EDIT,  /**< A single-line edit box is invoked when the user
                              activates the cell.\  The line edit is created
                              externally to the tree widget, and any
                              enhancements or restrictions are added by the
                              object creating the line edit.\  It is set into
                              the tree widget with the setCustomLineEdit()
                              method. */
      BROWSE_FILE_EDIT,  /**< A single-line edit box with a browse button is
                              invoked when the user activates the cell.\  When
                              the user clicks the browse button, a file
                              selection dialog is invoked to select a single
                              file. */
      BROWSE_DIR_EDIT,   /**< A single-line edit box with a browse button is
                              invoked when the user activates the cell.\  When
                              the user clicks the browse button, a file
                              selection dialog is invoked to select a
                              directory. */
      SHORTCUT_EDIT,     /**< A single-line edit box is invoked when the user
                              activates the cell.\  As the user types in the
                              edit box, the keystrokes are converted to
                              keyboard shortcut combinations. */
      COMBO_BOX,         /**< A drop-down combo box is invoked when the user
                              activates the cell.\  The combo box is created
                              and populated by the object creating the tree
                              widget.\  It is set into the tree widget with the
                              setComboBox() method. */
      SPIN_BOX,          /**< A spin box for integer values is invoked when the user
                              activates the cell.\  The spin box is created and its
                              step size and range of values are determined by the
                              object creating the tree widget.\  It is set into the
                              tree widget with the setSpinBox() method. */
      DOUBLE_SPIN_BOX,   /**< A spin box for double precision floating-point values is
                              invoked when the user activates the cell.\  The spin box
                              is created and its step size and range of values are determined
                              by the object creating the tree widget.\  It is set into the
                              tree widget with the setDoubleSpinBox() method. */
      CUSTOM_WIDGET      /**< A custom edit widget is invoked when the user activates the cell.\  The
                              widget must be derived from CustomEditWidget and provide implementations
                              for the pure virtual methods.\  The widget must be created by the object
                              creating the tree widget.\  It is set into the tree widget with the
                              setCustomEditWidget() method.*/
   };

   /**
    * @EnumWrapper CustomTreeWidget::WidgetTypeEnum.
    */
   typedef EnumWrapper<WidgetTypeEnum> WidgetType;

   /**
    *  Specifies the state of the edit check box in a cell of the tree widget.
    *
    *  @see        setCellCheckState()
    */
   enum CheckStateEnum
   {
      UNCHECKED = 0,  /**< The check box is not checked. */
      CHECKED,        /**< The check box is checked. */
      SEMI_CHECKED    /**< The check box is partially checked, indicating that
                           the check box represents multiple values, some of
                           which are selected and some are not selected. */
   };

   /**
    * @EnumWrapper CustomTreeWidget::CheckStateEnum.
    */
   typedef EnumWrapper<CheckStateEnum> CheckState;

   /**
    *  Deletes all tree widget items.
    *
    *  This method clears the tree widget by destroying all tree widget items,
    *  but the columns and their names and sizes are not changed.
    */
   virtual void clear();

   /**
    *  Displays a check box in a given cell and sets its check state.
    *
    *  @param   pItem
    *           The item for which to display the check box and set its value.
    *  @param   iColumn
    *           The item column in which to display the check box and set its
    *           value.
    *  @param   eState
    *           The check state to set as the cell value.
    *
    *  @return  Returns \c true if the check box was successfully displayed and
    *           the value set; otherwise returns \c false.
    */
   bool setCellCheckState(QTreeWidgetItem* pItem, int iColumn, CheckState eState);

   /**
    *  Returns the current state of the check box in the given cell.
    *
    *  @param   pItem
    *           The item for which to query a cell check state.
    *  @param   iColumn
    *           The item column to query its check state.
    *
    *  @return  The check state of the given cell.  CustomTreeWidget::UNCHECKED
    *           is returned if the cell is not displaying a check box.
    */
   CheckState getCellCheckState(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Displays a pixmap of a given color in a cell.
    *
    *  @param   pItem
    *           The item for which to display the color pixmap and set its
    *           value.
    *  @param   iColumn
    *           The item column in which to display the color pixmap and set
    *           its value.
    *  @param   clrCell
    *           The color to set as the cell value.  If an invalid color is
    *           given, the pixmap is removed from the cell.
    *
    *  @return  Returns \c true if the color pixmap is successfully displayed;
    *           otherwise returns \c false.
    */
   bool setCellColor(QTreeWidgetItem* pItem, int iColumn, QColor clrCell);

   /**
    *  Returns the current color of a given cell.
    *
    *  @param   pItem
    *           The item for which to query a cell color.
    *  @param   iColumn
    *           The item column to query its color.
    *
    *  @return  The current cell color.  An invalid color is returned if the
    *           cell is not currently displaying a color.
    */
   QColor getCellColor(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Toggles whether the size of a color pixmap adjusts to the column width.
    *
    *  If a cell in an item is displaying a color pixmap, this method toggles
    *  the format for the pixmap width.  The pixmap can either adjust to the
    *  column size, which automatically updates as the column is resized, or
    *  the pixmap can have a fixed width to allow text to be displayed next to
    *  the pixmap.
    *
    *  Calling this method affects all cells displaying a color pixmap and it
    *  has no effect if no cells currently display a color pixmap.
    *
    *  @param   bFullCell
    *           Set this value to true to automatically adjust the pixmap size
    *           to the column width.  Set this value to false to use a fixed
    *           size for the color pixmap.
    *
    *  @see     setCellColor(), setColorWidth()
    */
   void setFullCellColor(bool bFullCell);

   /**
    *  Queries whether the size of a color pixmap adjusts to the column width.
    *
    *  @return  Returns \c true if the width of color pixmaps adjust to the
    *           column width.  Returns \c false if the color pixmaps use a
    *           fixed width.
    */
   bool getFullCellColor() const;

   /**
    *  Sets a fixed width for cell color pixmaps.
    *
    *  This method sets a fixed width for all cells displaying a color pixmap.
    *  The default fixed width is 50 screen pixels.  Calling this method has no
    *  effect if no cells currently display a color pixmap, or if the pixmap is
    *  set to adjust to the column width.
    *
    *  @param   iWidth
    *           The width of the color pixmap.
    *
    *  @see     setFullCellColor()
    */
   void setColorWidth(int iWidth);

   /**
    *  Returns the fixed width for cell color pixmaps.
    *
    *  @return  The fixed width of the color pixmaps.  A valid value is
    *           returned regardless of whether any cells are currently
    *           displaying color pixmaps or whether the pixmap width adjusts to
    *           the column width.
    */
   int getColorWidth() const;

   /**
    *  Returns the current edit widget type of a given cell.
    *
    *  @param   pItem
    *           The item to get a cell edit widget type.
    *  @param   iColumn
    *           The item column to get its edit widget type.
    *
    *  @return  The cell edit widget type.
    */
   WidgetType getCellWidgetType(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Sets the current edit widget type of a given cell.
    *
    *  This method sets the current edit widget type for a cell.  The edit
    *  widget can be used in conjunction with a cell color and a cell check
    *  box.
    *
    *  @param   pItem
    *           The item to set a cell edit widget type. This item can either be present
    *           in the tree at the time this method is called or added at a later time.
    *  @param   iColumn
    *           The item column to set its edit widget type.
    *  @param   eType
    *           The edit widget type for the cell.
    */
   bool setCellWidgetType(QTreeWidgetItem* pItem, int iColumn, WidgetType eType);

   /**
    *  Toggles whether the cell edit widget covers the entire cell.
    *
    *  If a cell in an item contains an edit widget, this method toggles the
    *  format for the widget width.  The widget can either cover the entire
    *  cell or just the portion of the cell not covered by the cell icon.
    *
    *  The default behavior is for the edit widget to cover the entire cell
    *  area.
    *
    *  Calling this method has no effect if the cell is read-only.
    *
    *  @param   pItem
    *           The item to set a cell's edit widget width format.
    *  @param   iColumn
    *           The item column to set its edit widget width format.
    *  @param   bFullCell
    *           Set this value to \c true to automatically adjust the edit
    *           widget size to the column width.  Set this value to \c false
    *           for the edit widget to not cover the cell icon.
    */
   void setFullCellEdit(QTreeWidgetItem* pItem, int iColumn, bool bFullCell);

   /**
    *  Queries whether a cell edit widget covers the entire cell.
    *
    *  @param   pItem
    *           The item to query for a cell's edit widget width format.
    *  @param   iColumn
    *           The item column to query its edit widget width format.
    *
    *  @return  Returns \c true if the edit widget of the given cell covers
    *           the entire cell.  Returns \c false if the edit widget does not
    *           cover the cell icon..
    */
   bool getFullCellEdit(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Sets a custom line edit to use as the edit widget for a given cell.
    *
    *  This method sets a custom line edit as the edit widget for the given
    *  cell.  The cell edit widget type must be set to
    *  CustomTreeWidget::CUSTOM_LINE_EDIT before this method is called.
    *
    *  @param   pItem
    *           The item in which to set the custom line edit as a cell edit
    *           widget.
    *  @param   iColumn
    *           The item column in which to set the custom line edit as its edit
    *           widget.
    *  @param   pLineEdit
    *           The line edit to use as the edit widget.  The line edit is
    *           reparented to the viewport widget, so it will automatically be
    *           deleted when the tree widget is deleted.
    *
    *  @return  Returns \c true if the line edit was successfully set as the
    *           edit widget; otherwise returns \c false.
    *
    *  @see     setCellWidgetType()
    */
   bool setCustomLineEdit(QTreeWidgetItem* pItem, int iColumn, QLineEdit* pLineEdit);

   /**
    *  Returns the custom line edit used as the edit widget for a given cell.
    *
    *  @param   pItem
    *           The item in which to get a cell custom line edit widget.
    *  @param   iColumn
    *           The item column in which to get the custom line edit widget.
    *
    *  @return  The custom line edit widget.  A valid line edit pointer is
    *           returned regardless of the current edit widget if a line edit
    *           has been previously set as the edit widget but not reset to
    *           \c NULL.
    */
   QLineEdit* getCustomLineEdit(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Sets the file browser to use as the edit widget for a given cell.
    *
    *  This method sets a file browser as the edit widget for the given cell.
    *  The cell edit widget type must be set to
    *  CustomTreeWidget::BROWSE_FILE_EDIT before this method is called.
    *
    *  @param   pItem
    *           The item in which to set the file browser as a cell edit widget.
    *  @param   iColumn
    *           The item column in which to set the file browser as its edit
    *           widget.
    *  @param   pFileBrowser
    *           The file browser to use as the edit widget.  The file browser
    *           is reparented to the viewport widget, so it will automatically
    *           be deleted when the tree widget is deleted.
    *
    *  @return  Returns \c true if the file browser was successfully set as the
    *           edit widget; otherwise returns \c false.
    *
    *  @see     setCellWidgetType()
    */
   bool setFileBrowser(QTreeWidgetItem* pItem, int iColumn, FileBrowser* pFileBrowser);

   /**
    *  Returns the file browser used as the edit widget for a given cell.
    *
    *  @param   pItem
    *           The item in which to get a cell file browser edit widget.
    *  @param   iColumn
    *           The item column in which to get the file browser edit widget.
    *
    *  @return  The file browser edit widget.  A valid item is returned
    *           regardless of the current edit widget if a file browser has
    *           been previously set as the edit widget but not reset to \c NULL.
    */
   FileBrowser* getFileBrowser(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Sets the combo box to use as the edit widget for a given cell.
    *
    *  This method sets a combo box as the edit widget for the given cell.  The
    *  cell edit widget type must be set to CustomTreeWidget::COMBO_BOX before
    *  this method is called.
    *
    *  @param   pItem
    *           The item in which to set the combo box as a cell edit widget.
    *  @param   iColumn
    *           The item column in which to set the combo box as its edit
    *           widget.
    *  @param   pCombo
    *           The populated combo box to use as the edit widget.  The combo
    *           box is reparented to the viewport widget, so it will
    *           automatically be deleted when the tree widget is deleted.
    *
    *  @return  Returns \c true if the combo box was successfully set as the
    *           edit widget; otherwise returns \c false.
    *
    *  @see     setCellWidgetType()
    */
   bool setComboBox(QTreeWidgetItem* pItem, int iColumn, QComboBox* pCombo);

   /**
    *  Returns the combo box used as the edit widget for a given cell.
    *
    *  @param   pItem
    *           The item in which to get a cell combo box edit widget.
    *  @param   iColumn
    *           The item column in which to get the combo box edit widget.
    *
    *  @return  The combo box edit widget.  A valid item is returned regardless
    *           of the current edit widget if a combo box has been previously
    *           set as the edit widget but not reset to \c NULL.
    */
   QComboBox* getComboBox(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Sets the spin box to use as the edit widget for a given cell.
    *
    *  This method sets a spin box as the edit widget for the given cell.  The
    *  cell edit widget type must be set to CustomTreeWidget::SPIN_BOX before this method is
    *  called.
    *
    *  @param   pItem
    *           The item in which to set the spin box as a cell edit widget.
    *  @param   iColumn
    *           The item column in which to set the spin box as its edit
    *           widget.
    *  @param   pSpin
    *           The spin box to use as the edit widget.  The spin box is
    *           reparented to the viewport widget, so it will automatically be
    *           deleted when the tree widget is deleted.
    *
    *  @return  Returns \c true if the spin box was successfully set as the
    *           edit widget; otherwise returns \c false.
    *
    *  @see     setDoubleSpinBox(), setCellWidgetType()
    */
   bool setSpinBox(QTreeWidgetItem* pItem, int iColumn, QSpinBox* pSpin);

   /**
    *  Returns the spin box used as the edit widget for a given cell.
    *
    *  @param   pItem
    *           The item in which to get a cell spin box edit widget.
    *  @param   iColumn
    *           The item column in which to get its spin box edit widget.
    *
    *  @return  The spin box edit widget.  A valid item is returned regardless
    *           of the current edit widget if a spin box has been previously
    *           set as the edit widget but not reset to \c NULL.
    *
    *  @see     getDoubleSpinBox()
    */
   QSpinBox* getSpinBox(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Sets the double spin box to use as the edit widget for a given cell.
    *
    *  This method sets a double spin box as the edit widget for the given cell.  The
    *  cell edit widget type must be set to CustomTreeWidget::DOUBLE_SPIN_BOX before this method is
    *  called.
    *
    *  @param   pItem
    *           The item in which to set the double spin box as a cell edit widget.
    *  @param   iColumn
    *           The item column in which to set the double spin box as its edit
    *           widget.
    *  @param   pSpin
    *           The double spin box to use as the edit widget.  The double spin box is
    *           reparented to the viewport widget, so it will automatically be
    *           deleted when the tree widget is deleted.
    *
    *  @return  Returns \c true if the double spin box was successfully set as the
    *           edit widget; otherwise returns \c false.
    *
    *  @see     setSpinBox(), setCellWidgetType()
    */
   bool setDoubleSpinBox(QTreeWidgetItem* pItem, int iColumn, QDoubleSpinBox* pSpin);

   /**
    *  Returns the double spin box used as the edit widget for a given cell.
    *
    *  @param   pItem
    *           The item in which to get a cell double spin box edit widget.
    *  @param   iColumn
    *           The item column in which to get its double spin box edit widget.
    *
    *  @return  The double spin box edit widget.  A valid item is returned regardless
    *           of the current edit widget if a double spin box has been previously
    *           set as the edit widget but not reset to \c NULL.
    *
    *  @see     getSpinBox()
    */
   QDoubleSpinBox* getDoubleSpinBox(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Sets a custom edit widget to use as the edit widget for a given cell.
    *
    *  This method sets a custom edit widget as the edit widget for the given
    *  cell.  The cell edit widget type must be set to
    *  CustomTreeWidget::CUSTOM_WIDGET before this method is called.
    *
    *  @param   pItem
    *           The item in which to set the custom edit widget as a cell edit
    *           widget.
    *  @param   iColumn
    *           The item column in which to set the custom edit widget as its edit
    *           widget.
    *  @param   pCustomEditWidget
    *           The custom edit widget to use as the edit widget.  The custom edit widget
    *           is reparented to the viewport widget, so it will automatically be
    *           deleted when the tree widget is deleted.
    *
    *  @return  Returns \c true if the custom edit widget was successfully set as the
    *           edit widget; otherwise returns \c false.
    *
    *  @see     setCellWidgetType()
    */
   bool setCustomEditWidget(QTreeWidgetItem* pItem, int iColumn, CustomEditWidget* pCustomEditWidget);

   /**
    *  Returns the custom edit widget used as the edit widget for a given cell.
    *
    *  @param   pItem
    *           The item in which to get a cell custom edit widget.
    *  @param   iColumn
    *           The item column in which to get the custom edit widget.
    *
    *  @return  The custom edit widget. A valid pointer is returned regardless
    *           of the current edit widget if a custom edit widget has been
    *           previously set as the edit widget but not reset to \c NULL.
    */
   CustomEditWidget* getCustomEditWidget(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Invokes the edit widget for a given cell.
    *
    *  This method invokes the edit widget for a given cell.  It does nothing
    *  if the cell edit widget type is CustomTreeWidget::NO_WIDGET.
    *
    *  @param   pItem
    *           The item in which to invoke a cell edit widget.
    *  @param   iColumn
    *           The item column in which to invoke its edit widget.
    *
    *  @see     CellType
    */
   void activateCellWidget(QTreeWidgetItem* pItem, int iColumn);

   /**
    *  Tests if a given point is contained within a given item cell.
    *
    *  @param   ptCoord
    *           The screen pixel coordinate to test.
    *  @param   pItem
    *           The item to test for the coordinate.
    *  @param   iColumn
    *           The item column to test for the coordinate.
    *
    *  @return  Returns \c true if the given coordinate is contained within the
    *           given item cell, otherwise returns \c false.
    */
   bool hitTest(QPoint ptCoord, QTreeWidgetItem* pItem, int iColumn);

   /**
    *  Toggles the display of horizontal or vertical gridlines.
    *
    *  @param   orientations
    *           Indicates whether to show or hide horizontal or vertical
    *           gridlines, or both.
    *  @param   bShow
    *           Set this value to \c true to display the gridlines, or to
    *           \c false to hide the gridlines.
    */
   void setGridlinesShown(Qt::Orientations orientations, bool bShow);

   /**
    *  Queries whether gridlines are currently displayed.
    *
    *  @param   orientation
    *           Indicates whether to show or hide either horizontal or vertical
    *           gridlines.
    *
    *  @return  Returns \c true if gridlines are displayed for the given
    *           orientation; otherwise returns \c false.
    */
   bool areGridlinesShown(Qt::Orientation orientation) const;

   /**
    *  Performs the keyboard shortcut conversion for the
    *  CustomTreeWidget::SHORTCUT_EDIT edit widget type.
    *
    *  @param   pObject
    *           The object prompting the event.
    *  @param   pEvent
    *           The event invoked by the object.
    *
    *  @return  Returns \c true if a keyboard shortcut conversion was
    *           successfully performed; otherwise returns \c false.
    */
   bool eventFilter(QObject* pObject, QEvent* pEvent);

public slots:
   /**
    *  Closes the active edit widget.
    *
    *  This method closes the active edit widget if it is currently invoked.
    *  It is often useful to call this method if the list view is contained
    *  inside a dialog and the user has clicked on the OK or Cancel buttons.
    *
    *  @param   bAcceptEdit
    *           Set this value to \c true to accept any changes the user made
    *           in the edit widget before closing.  Set this value to \c false
    *           to ignore any user changes.
    */
   void closeActiveCellWidget(bool bAcceptEdit);

   /**
    *  Sets the initial browse directory.
    *
    *  This method pertains to the initial browse directory when the user
    *  clicks on the browse button for the CustomTreeWidget::BROWSE_DIR_EDIT
    *  edit widget type.
    *
    *  @param   strDirectory
    *           The directory to set as the initial browse directory.  Set this
    *           value to an empty string to use the current working directory.
    *
    *  @see     setCellWidgetType()
    */
   void setBrowseDir(const QString& strDirectory);

signals:
   /**
    *  This signal is emitted when the text of a cell is changed.
    *
    *  @param   pItem
    *           The item with modified text.
    *  @param   iColumn
    *           The item column containing the text that has changed.
    */
   void cellTextChanged(QTreeWidgetItem* pItem, int iColumn);

   /**
    *  This signal is emitted when the pixmap of a cell is changed.
    *
    *  @param   pItem
    *           The item with a modified pixmap.
    *  @param   iColumn
    *           The item column containing the pixmap that has changed.
    */
   void cellIconChanged(QTreeWidgetItem* pItem, int iColumn);

   /**
    *  This signal is emitted when the color of a cell is changed.
    *
    *  @param   pItem
    *           The item with a modified color.
    *  @param   iColumn
    *           The item column containing the color that has changed.
    */
   void cellColorChanged(QTreeWidgetItem* pItem, int iColumn);

   /**
    *  This signal is emitted when the check box state of a cell is changed.
    *
    *  @param   pItem
    *           The item with a modified check box state.
    *  @param   iColumn
    *           The item column containing the check box whose state has
    *           changed.
    */
   void cellCheckChanged(QTreeWidgetItem* pItem, int iColumn);

   /**
    *  This signal is emitted when the delete key is pressed.
    */
   void deleteKeyPressed();

protected:
   /**
    *  Invokes the cell edit widget.
    *
    *  This method is called by Qt when the user clicks inside the tree widget
    *  area.  If an item cell is clicked and the item is selected, the edit
    *  widget is invoked.  If the user clicked on a color pixmap, a common
    *  color selected dialog is invoked instead.  If the user clicked on the
    *  check box, the check state is toggled.
    *
    *  @param   e
    *           The mouse event associated with the mouse press.
    */
   void mousePressEvent(QMouseEvent* e);

   /**
    *  Invokes the cell edit widget.
    *
    *  This method is called by Qt when the user double clicks inside the tree
    *  widget area.  If the item that is double clicked is not selected, it is
    *  selected and the edit widget is invoked by calling mousePressEvent().
    *
    *  @param   e
    *           The mouse event associated with the mouse double click.
    */
   void mouseDoubleClickEvent(QMouseEvent* e);

   /**
    *  Performs common operation in response to key presses.
    *
    *  This method is called by Qt when the user presses a key on the keyboard.
    *  The default behavior is as follows:
    *
    *  <pre>
    *  Key            Behavior
    *  =============  =========================================================
    *  Delete         Emits the deleteKeyPressed() signal.
    *  Esc            Closes the active edit widget and ignores any changes.
    *  Return/Enter   Closes the active edit widget and accepts any changes.
    *  Down %Arrow     Closes the active edit widget and accepts any changes.
    *                 Selects the item below the currently selected item and
    *                 activates the edit widget in the same column as the edit
    *                 widget that was closed.
    *  Up %Arrow       Closes the active edit widget and accepts any changes.
    *                 Selects the item above the currently selected item and
    *                 activates the edit widget in the same column as the edit
    *                 widget that was closed.
    *  Page Down      Closes the active edit widget and accepts any changes.
    *                 Scrolls the list view to display items below the
    *                 previously displayed items.
    *  Page Up        Closes the active edit widget and accepts any changes.
    *                 Scrolls the list view to display items above the
    *                 previously displayed items.
    *  </pre>
    *
    *  @param   e
    *           The key event associated with the key press.
    *
    *  @see     closeActiveCellWidget()
    */
   void keyPressEvent(QKeyEvent* e);

   /**
    *  Returns the index of the column at a given screen x-coordinate.
    *
    *  @param   iItemX
    *           The screen x-coordinate value for which to get the column
    *           index.
    *
    *  @return  The index of the column at the given screen x-coordinate.
    */
   int getColumnIndex(int iItemX) const;

   /**
    *  Returns the bounding rectangle of the given item cell.
    *
    *  The cell bounding rectangle is a combination of the pixmap rectangle
    *  and the edit widget rectangle.
    *
    *  @param   pItem
    *           The item containing the cell for which to get its rectangle.
    *  @param   iColumn
    *           The item column identifying the cell for which to get its
    *           rectangle.
    *
    *  @return  The cell bounding rectangle.
    */
   QRect getCellRect(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Returns the rectangle of the pixmap in a given item cell.
    *
    *  @param   pItem
    *           The item containing the cell for which to get the pixmap
    *           rectangle.
    *  @param   iColumn
    *           The item column identifying the cell for which to get its
    *           pixmap rectangle.
    *
    *  @return  The pixmap rectangle of the given cell.
    */
   QRect getCellPixmapRect(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Returns the rectangle of the edit widget in a given item cell.
    *
    *  @param   pItem
    *           The item containing the cell for which to get the edit widget
    *           rectangle.
    *  @param   iColumn
    *           The item column identifying the cell for which to get its edit
    *           widget rectangle.
    *
    *  @return  The pixmap rectangle of the given cell.
    */
   QRect getCellWidgetRect(QTreeWidgetItem* pItem, int iColumn) const;

   /**
    *  Draws the gridlines for the row if necessary.
    *
    *  This method overrides the QTreeWidget base class implementation to draw
    *  the gridlines on each cell in the row if necessary.
    *
    *  @param   pPainter
    *           The painter object in which to draw the cell in the row.
    *  @param   option
    *           The style in which the cell should be drawn.
    *  @param   index
    *           The index of the cell to draw.
    *
    *  @see     setGridlinesShown()
    */
   void drawRow(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected slots:
   /**
    *  Resizes the edit widget and/or the color pixmap.
    *
    *  This method is called when the user resizes a column.  The color pixmap
    *  is resized if fixed widths are not used.  %Any active edit widget is
    *  also resized to fit within the bounding rectangle of the cell.
    *
    *  @param   iColumn
    *           The item column whose width has changed.
    *  @param   iOldWidth
    *           The previous width of the item column.
    *  @param   iNewWidth
    *           The new width of the item column.
    */
   void columnWidthChanged(int iColumn, int iOldWidth, int iNewWidth);

   /**
    *  Sets the text of the current cell.
    *
    *  This method is called when the user accepts changes in the active edit
    *  widget.
    *
    *  @param   strText
    *           The new text for the cell.
    */
   void setCurrentCellText(const QString& strText);

   /**
    *  Invokes a directory selection dialog to select a directory.
    *
    *  This method is called when the user clicks on the browse button from
    *  the active edit widget to select a directory.
    */
   void browse();

   /**
    *  Accepts the changes in the active line edit widget.
    *
    *  This method sets the cell text to the current text in the active line
    *  edit widget.
    */
   void acceptEditText();

   /**
    *  Hides the active line edit widget.
    *
    *  @see     closeActiveCellWidget()
    */
   void closeEdit();

   /**
    *  Accepts the changes in the active custom line edit widget.
    *
    *  This method sets the cell text to the current text in the active custom
    *  line edit widget.
    */
   void acceptCustomEditText();

   /**
    *  Hides the active custom line edit widget.
    *
    *  @see     closeActiveCellWidget()
    */
   void closeCustomEdit();

   /**
    *  Accepts the changes in the active file browser edit widget.
    *
    *  This method sets the cell text to the current text in the active file
    *  browser edit widget.
    */
   void acceptFileBrowserText();

   /**
    *  Hides the active file browser edit widget.
    *
    *  @see     closeActiveCellWidget()
    */
   void closeFileBrowser();

   /**
    *  Accepts the changes in the active combo box edit widget.
    *
    *  This method sets the cell text to the current text in the active combo
    *  box edit widget.
    */
   void acceptComboText();

   /**
    *  Hides the active combo box edit widget.
    *
    *  @see     closeActiveCellWidget()
    */
   void closeCombo();

   /**
    *  Accepts the changes in the active spin box edit widget.
    *
    *  This method sets the cell text to the current value in the active spin
    *  box edit widget.
    */
   void acceptSpinText();

   /**
    *  Hides the active spin box edit widget.
    *
    *  @see     closeActiveCellWidget()
    */
   void closeSpin();

   /**
    *  Accepts the changes in the active double spin box edit widget.
    *
    *  This method sets the cell text to the current value in the active double spin
    *  box edit widget.
    */
   void acceptDoubleSpinText();

   /**
    *  Hides the active double spin box edit widget.
    *
    *  @see     closeActiveCellWidget()
    */
   void closeDoubleSpin();

   /**
    *  Accepts the changes in the active custom edit widget.
    *
    *  This method sets the cell text to the current text in the active custom
    *  edit widget.
    */
   void acceptCustomWidgetText();

   /**
    *  Hides the active custom edit widget.
    *
    *  @see     closeActiveCellWidget()
    */
   void closeCustomWidget();

private:
   CustomTreeWidget(const CustomTreeWidget& rhs);
   CustomTreeWidget& operator=(const CustomTreeWidget& rhs);

   class CellLocation
   {
   public:
      CellLocation() :
         pItem(NULL),
         iColumn(-1)
      {
      }

      bool operator<(const CellLocation& cell) const
      {
         if (cell.pItem < pItem)
         {
            return true;
         }
         else if (cell.pItem > pItem)
         {
            return false;
         }
         else if (cell.iColumn < iColumn)
         {
            return true;
         }

         return false;
      };

      QTreeWidgetItem* pItem;
      int iColumn;
   };

   class CustomTreeWidgetItemDelegate : public QStyledItemDelegate
   {
   public:
      CustomTreeWidgetItemDelegate()
      {}

      QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
      {
         QSize itemSize = QStyledItemDelegate::sizeHint(option, index);
         itemSize.setHeight(20);

         return itemSize;
      }
   private:
      CustomTreeWidgetItemDelegate(const CustomTreeWidgetItemDelegate& rhs);
      CustomTreeWidgetItemDelegate& operator=(const CustomTreeWidgetItemDelegate& rhs);
   };

   bool mbFullColor;
   int miColorWidth;
   bool mHorizontalGridlines;
   bool mVerticalGridlines;

   QLineEdit* mpEdit;
   QLineEdit* mpCustomEdit;
   FileBrowser* mpFileBrowser;
   QPushButton* mpBrowse;
   QString mBrowseDir;
   QComboBox* mpCombo;
   QSpinBox* mpSpin;
   QDoubleSpinBox* mpDoubleSpin;
   QKeySequence mShortcut;
   CustomEditWidget* mpCustomWidget;

   QMap<CellLocation, WidgetType> mCellWidgets;
   QMap<CellLocation, QLineEdit*> mCustomLineEdits;
   QMap<CellLocation, FileBrowser*> mFileBrowsers;
   QMap<CellLocation, QComboBox*> mComboBoxes;
   QMap<CellLocation, QSpinBox*> mSpinBoxes;
   QMap<CellLocation, QDoubleSpinBox*> mDoubleSpinBoxes;
   QMap<CellLocation, CheckState> mChecks;
   QMap<CellLocation, ColorType> mColors;
   QMap<CellLocation, bool> mFullCellEdit;
   QMap<CellLocation, CustomEditWidget*> mCustomEditWidgets;
};

/**
 *  A base class for specialized in-cell editing capabilities in a CustomTreeWidget that can't be implemented
 *  using the provided edit widget types.
 *
 *  A specialized edit widget must be derived from CustomEditWidget and provide implementations for text() and
 *  setText(). Optionally, it should override selectAll() if appropriate.
 */
class CustomEditWidget : public QWidget
{
   Q_OBJECT

public:
   /**
    *  Creates a new, empty custom edit widget.
    *
    *  @param   pParent
    *           The parent widget.
    */
   CustomEditWidget(QWidget* pParent = NULL);

   /**
    *  Destroys the custom edit widget and all child widgets.
    */
   virtual ~CustomEditWidget();

   /**
    *  Retrieves the edited text from the custom edit widget.
    *
    *  @return  The edited text currently associated with the custom edit widget.
    */
   virtual QString text() const = 0;

public slots:
   /**
    *  Sets the text to be edited into the custom edit widget.
    *
    *  @param   text
    *           The text to be edited.
    */
   virtual void setText(const QString& text) = 0;
   /**
    *  Selects all the text in the custom edit widget.
    *
    *  @note    This class provides only an empty implementation. If this capability is
    *           required, the derived class must provide an implementation.
    */
   virtual void selectAll() {}
};

#endif
