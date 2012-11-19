/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPEDITORDLG_H
#define GCPEDITORDLG_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QDialog>

#include "GcpList.h"
#include "Observer.h"
#include "TypesFile.h"

#include <list>

class CustomTreeWidget;
class DmsFormatTypeComboBox;
class GcpLayer;
class GeocoordTypeComboBox;
class Layer;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QTreeWidgetItem;

namespace boost
{
   class any;
}

/**
 *  Displays the GCPs of a GCP layer.
 *
 *  The GcpEditorDlg class is a dialog that provides an interface to view and
 *  modify the GCPs in a GCP layer.  The active layer can be set with the
 *  setGcpList() method.
 */
class GcpEditorDlg : public QDialog, public Observer
{
   Q_OBJECT

public:
   /**
    *  Creates the GCP editor as a modeless dialog.
    *
    *  @param    parent
    *            The parent widget for the dialog.
    */
   GcpEditorDlg(QWidget* parent = 0);

   /**
    *  Destroys the GCP editor.
    */
   virtual ~GcpEditorDlg();

public slots:
   /**
    *  Sets the active GCP list layer in the editor.
    *
    *  This method specifies the GCP layer in which to display its GCPs.  The tree
    *  widget is cleared if the active GCP list contains no points.
    *
    *  @param   pLayer
    *           The GCP layer for which to display its GCPs.
    */
   bool setGcpLayer(Layer* pLayer);

   /**
    *  Adds a new GCP layer to the list of available layers.
    *
    *  @param   pLayer
    *           The new GCP layer to add to the list.  Cannot be NULL.
    *
    *  @return  True if the GCP layer was successfully added, otherwise false.
    */
   bool addLayer(Layer* pLayer);

   /**
    *  Removes a GCP layer from the list of available layers.
    *
    *  @param   pLayer
    *           The GCP layer to remove from the list.  Cannot be NULL.
    *
    *  @return  True if the GCP layer was successfully removed, otherwise false.
    */
   bool removeLayer(Layer* pLayer);

   /**
    *  Updates the list of available GCP layers to those of the current data set.
    */
   void updateLayers();

signals:
   /**
    *  Indicates that the editor is shown or hidden.
    *
    *  @param   bVisible
    *           TRUE if the dialog has been shown, FALSE if the dialog has been hidden.
    */
   void visibilityChanged(bool bVisible);

   /**
    *  Indicates that a new GCP layer was activated from the list.
    *
    *  @param   pLayer
    *           The new GCP layer that was activated.
    */
   void layerActivated(Layer* pLayer);

protected:
   /**
    *  Prompts the user to apply changes to the GCP list.
    *
    *  This method is called automatically by Qt.  When the user clicks outside
    *  of the GCP editor, this method checks if changes have been made to the
    *  GCP list and prompts the user to apply the changes if necessary.  If the
    *  auto-apply box is checked, any changes are applied automatically.
    *
    *  @param   pEvent
    *           The event that occurred on the dialog.  All event types other
    *           than QEvent::WindowDeactivate are ignored.
    *
    *  @return  Returns true if the event was processed, otherwise false.
    */
   bool event(QEvent* pEvent);

   /**
    *  Emits the signal to indicate that the dialog has been shown.
    *
    *  @param   e
    *           The show message containing information about the event.
    *
    *  @see     GcpEditorDlg::visibilityChanged
    */
   void showEvent(QShowEvent* e);

   /**
    *  Emits the signal to indicate that the dialog has been closed.
    *
    *  @param   e
    *           The close message containing information about the event.
    *
    *  @see     GcpEditorDlg::visibilityChanged
    */
   void closeEvent(QCloseEvent* e);

   void attached(Subject& subject, const std::string& signal, const Slot& slot);
   void elementModified(Subject& subject, const std::string& signal, const boost::any& value);
   void optionsModified(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   /**
    *  Activates a GCP layer according to its index in the list of available
    *  layers.
    *
    *  @param   iIndex
    *           The list index of the GCP layer to activate.
    */
   void setActiveLayer(int iIndex);

   /**
    *  Updates the names of all layers in the available layers list.
    */
   void updateLayerNames();

   /**
    *  Removes all GCP layers from the list of available layers.
    */
   void clearLayers();

   /**
    *  Enables the GCP widgets based on the number of available GCP layers in the list.
    */
   void enableGcp();

   /**
    *  Refreshes the tree widget to display the given GCPs.
    *
    *  The GCPs are displayed in the tree widget with an item for each GCP.  The
    *  geocoordinate type is determined by the currently selected format, and the
    *  appropriate columns are displayed for the geocoordinate.
    *
    *  @param   gcps
    *           The list of GCPs to display in the tree widget.
    */
   void updateGcpView(const std::list<GcpPoint>& gcps);

   /**
    *  Reformats the cell text after it has been edited.
    *
    *  The cell will be displayed in the appropriate coordinate system.  It reads
    *  the entered value, converts it to a geocoorinate, and then adds formatting
    *  to the text.
    *
    *  @param    pItem
    *            The tree widget item containing the cell that was changed.
    *  @param    iColumn
    *            The column of the cell that was changed.
    */
   void updateGcp(QTreeWidgetItem* pItem, int iColumn);

   /**
    *  Adds a new empty GCP to the table.
    *
    *  This methods adds a new GCP item to the table.  The GCP will have a pixel
    *  and geocoord of (0, 0). It is called when the user clicks the New button.
    */
   void newGcp();

   /**
    *  Deletes the selected GCPs.
    *
    *  This method is called when the user clicks the Delete button.
    */
   void deleteGcp();

   /**
    *  Sets the current geocoordinate display format of the GCPs.
    *
    *  This method is called when the user selects a new geocoordinate display
    *  type.  The GCP table is automatically updated to show the coordinates in
    *  the new type.
    *
    *  @param   geocoordType
    *           The new geocoordinate display format.
    */
   void setCoordinateFormat(GeocoordType geocoordType);

   /**
    *  Sets the current latitude/longitude display format of the GCPs.
    *
    *  This method is called when the user selects a new latitude/longitude
    *  display format.  The GCP table is automatically updated to show the
    *  latitude/longitude coordinates in the new format.
    *
    *  This method will still succeed if the current coordinate type is not
    *  Latitude/Longitude, however the GCP values will not update.
    *
    *  @param   latLonFormat
    *           The new latitude/longitude display format.
    */
   void setLatLonFormat(DmsFormatType latLonFormat);

   /**
    *  Invokes the properties dialog for the user to change the layer properties.
    */
   void setGcpProperties();

   /** 
    *  Applies the changes made in the editor to the active GCP list.
    *
    *  The GCP list is redrawn with the new information. If georeferencing had
    *  been performed on the GCP list, the results layer is not updated. The
    *  georeferencing plug-in must be re-run to update the LatLonLayer.  This
    *  method is called automatically when the user clicks the Apply button.
    */
   void apply();

private:
   GcpEditorDlg(const GcpEditorDlg& rhs);
   GcpEditorDlg& operator=(const GcpEditorDlg& rhs);
   QTreeWidgetItem* insertGcp(const GcpPoint& point);

   QComboBox* mpListCombo;
   CustomTreeWidget* mpGcpView;
   QPushButton* mpNewButton;
   QPushButton* mpDeleteButton;
   QLabel* mpCoordTypeLabel;
   GeocoordTypeComboBox* mpCoordTypeCombo;
   QLabel* mpLatLonFormatLabel;
   DmsFormatTypeComboBox* mpLatLonFormatCombo;
   QCheckBox* mpAutoApply;
   QPushButton* mpPropertiesButton;
   QPushButton* mpApplyButton;

   QList<GcpLayer*> mGcpLayers;
   GcpLayer* mpLayer;
   QMap<QTreeWidgetItem*, GcpPoint> mEditGcps;

   bool mbModified;
};

#endif
