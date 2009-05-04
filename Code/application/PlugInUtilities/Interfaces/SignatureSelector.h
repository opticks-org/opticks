/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef SIGNATURESELECTOR_H
#define SIGNATURESELECTOR_H

#include <QtCore/QStringList>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include "ModelServices.h"
#include "ObjectFactory.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"

#include <map>
#include <vector>

class Progress;
class SearchDlg;
class Signature;

/**
 *  A dialog to import and select signatures.
 *
 *  The signature selector is a dialog that allows users to select one or more Signature
 *  objects from a list.  The dialog displays a list of signatures that the user can
 *  adjust to filter out some of the signature methods.  The available filters are as
 *  follows:
 *  - <b>All Signatures:</b>  All Signature objects loaded into the data model are
 *    displayed.
 *  - <b>%AOI:</b>  All AOI objects loaded into the data model are displayed, since a
 *    Signature object can be derived from an AOI object.
 *  - <b>Metadata:</b>  Signature objects loaded into the data model that pass user
 *    specified metadata search parameters are displayed.
 *
 *  The first two filters are automatically populated when the filter is activated.  If
 *  the user does not find the desired signature(s) in the list, an Import button is
 *  provided for the user to import one or more signatures while the dialog remains
 *  active.
 *
 *  When the user clicks the Import button, the dialog is expanded to contain a separate
 *  list of signature files from which the user can select one or more signatures to
 *  import.  After import, the main signature list is updated appropriately.
 *
 *  On signature import, the user can browse the disk for one or more signature files in
 *  a single directory or the user can search one or more directories for recognized
 *  signature files.  The user can also apply a metadata filter when searching directories
 *  for signatures to import.
 *
 *  When a single signature in the main signature list is selected, the user can view
 *  the signature's properties by clicking on the Properties button in the dialog.
 *
 *  The dialog contains an Apply button so that the dialog can be modal or modeless as
 *  necessary.  The Apply button is only added to a modeless instantiation of the dialog.
 *
 *  @see        AoiElement, Signature
 */
class SignatureSelector : public QDialog
{
   Q_OBJECT

public:
   /**
    *  Creates the signature selector dialog.
    *
    *  The constructor creates the widgets and initializes signature list based on the
    *  "All Signatures" filter.
    *
    *  @param   pProgress
    *           An optional Progress object that is used when searching for signatures
    *           to import.
    *  @param   parent
    *           The parent widget.
    *  @param   mode
    *           The selection mode to be used for the list of signatures.
    *  @param   addApply
    *           If \c true, an Apply button will appear.
    *  @param   customButtonLabel
    *           Label that will appear on the custom button. The custom button will only be
    *           added to the dialog if \c customButtonLabel is not empty.
    */
   SignatureSelector(Progress* pProgress, QWidget* parent = 0,
      QAbstractItemView::SelectionMode mode = QAbstractItemView::ExtendedSelection, bool addApply = false,
      const std::string& customButtonLabel = std::string());

   /**
    *  Destroys the signature selector dialog.
    */
   ~SignatureSelector();

   /**
    *  Returns a vector of currently selected signatures.
    *
    *  This method returns a vector of all selected signature objects in the list view.
    *  If a signature set is selected, only the SignatureSet object is added to the
    *  vector.  To obtain a vector of all Signature objects inside a selected SignatureSet
    *  object, use the getExtractedSignatures() method instead.
    *
    *  @return  A vector of the selected Signature objects.
    */
   virtual std::vector<Signature*> getSignatures() const;

   /**
    *  Returns a vector of currently selected signatures.
    *
    *  This method returns a vector of each individual Signature object based on the
    *  selected items in the list view.  If a signature set is selected, the SignatureSet
    *  object is not added to the vector, but all individual Signature objects inside the
    *  SignatureSet object.  To obtain a vector of just the selected signature objects,
    *  use the getSignatures() method instead.
    *
    *  @return  A vector containing the individual Signature objects from the selected
    *           signature items in the list view.
    */
   std::vector<Signature*> getExtractedSignatures() const;

   /**
    *  Queries whether the Apply button is currently enabled.
    *
    *  The dialog does not automatically enable or disable the Apply button.  By default
    *  the Apply button is enabled when it is created.  So, this method is really only
    *  useful after the enableApplyButton() method has been called.
    *
    *  @return  This method returns \c true if the Apply button is currently enabled, or
    *           \c false if the Apply button is disabled.  \c False is also returned if the
    *           dialog is modal and therefore does not contain the Apply button.
    */
   bool isApplyButtonEnabled() const;

public slots:
   /**
    *  Stops the directory search for signatures to import.
    *
    *  This method is typically used as a slot method to connect or a cancel button on
    *  a progress dialog.  The method simply registers an abort notification with the
    *  search dialog, regardless of whether a search is currently in progress.  If this
    *  method is called independently from a user-instantiated event such as a button
    *  click, the abort will be registered and afterward if the user attempts a search,
    *  the abort will prevent that search from occurring.
    */
   void abortSearch();

signals:
   /**
    *  Indicates a change of the selected signatures in the list view.
    */
   void selectionChanged();

protected:
   /**
    *  Add an entry to the signature format combo box.
    *
    *  This is used by subclasses to add custom signature types. In addition to
    *  calling this method, a subclass will usually need to implement updateSignatureList() to
    *  populate the signature list when the custom type is selected. Second, getSignatures() will
    *  need to be implemented to return the selected signature when the custom type is selected.
    *
    *  @param type
    *         The type name to add.
    */
   void addCustomType(const QString &type);

   /**
    *  Access the current signature format type.
    *
    *  @return The current signature format type.
    */
   QString getCurrentFormatType() const;

   /**
    *  Access the signature list.
    *  Used to add items to the signature list when a custom format type is selected.
    *
    *  @return The signature list.
    */
   QTreeWidget *getSignatureList() const;

   /**
    *  Returns the widget layout in the dialog.
    *
    *  This method returns the layout in the dialog, which can be used by derived classes
    *  to add their own custom widgets.
    *
    *  @return  The layout for the child widgets in the dialog.
    */
   QGridLayout* getLayout() const;

   /**
    *  Sets the column text of the signature name column in the list view selection widget.
    *
    *  @param   strName
    *           The new name for the signature name column.  This method does nothing if
    *           the given string is empty.
    */
   void setNameText(const QString& strName);

   /**
    *  Returns the number of selected items in the list view.
    *
    *  This method returns the current number of selected items in the list view.  This
    *  does not necessarily equal the number of selected signatures returned by
    *  getSignatures().size().
    *
    *  @return  The number of selected signature items in the list view.
    */
   int getNumSelectedSignatures() const;

   /**
    *  Enables or disables the Apply button.
    *
    *  This method enables or disables the Apply button if the dialog is a modeless dialog.
    *  This method does nothing if the dialog is created as a modal dialog.
    *
    *  @param   enable
    *           Set this parameter to \c true to enable the Apply button or to \c false to
    *           disable the Apply button.
    *
    *  @see     isApplyButtonEnabled()
    */
   void enableApplyButton(bool enable);

   /**
   *  Enables or disables the custom button.
   *
   *  This method enables or disables the custom button. It allows a derived class to control
   *  enabling/disabling of the custom button. The button is initially enabled by default.
   *
   *  @param   enable
   *           Set this parameter to \c true to enable the custom button or to \c false to
   *           disable the custom button.
   */
   void enableCustomButton(bool enable);

protected slots:
   /**
    *  Applies actions based on the selected signatures.
    *
    *  This method is called if the dialog is modeless and the user clicks the Apply button
    *  or the OK button.  The default implementation of the method does nothing.  It is
    *  provided for derived classes to override to perform actions when the user clicks the
    *  button.  If a modeless dialog is created without the need for an Apply button, the
    *  button should be disabled.
    *
    *  @see     enableApplyButton()
    */
   virtual void apply();

   /**
    *  Sets the display filter for the signature list view.
    *
    *  This method updates the signature list view to display different signature items.  See
    *  the SignatureSelector class documentation for a description of the items that are
    *  displayed in each type.
    *
    *  @param   strFormat
    *           The new format type for the signature list view.  Valid strings are as follows:
    *           - "All Signatures"
    *           - "AOI"
    *           - "Metadata..."
    */
   void setDisplayType(const QString& strFormat);

   /**
    *  Enables or disables the Properties and Unload buttons.
    *
    *  This method enables or disabled the Properties and Unload buttons based on the number
    *  of selected signature items in the list view.
    */
   void enableButtons();

   /**
    *  Invokes a properties dialog for the currently selected signature.
    *
    *  This method invokes an instance of a SignaturePropertiesDlg for the first selected
    *  signature item in the list view.  This method is called automatically when the user
    *  clicks the Properties button.
    */
   void displaySignatureProperties();

   /**
    *  Export currently selected signatures as a signature set.
    */
   void exportSignatures();

   /**
    *  Destroys currently selected signatures.
    *
    *  This method destroys all Signature objects in the data model from all currently
    *  selected signature items in the list view.  This method is called automatically when
    *  the user clicks the Unload button.
    */
   void unloadSignatures();

   /**
    *  Toggles the display of the import extension widget.
    *
    *  This method toggles the import extension widget, which changes the overall size of
    *  the dialog.  This method is called automatically when the user clicks the Import
    *  button.
    */
   void importSignatures();

   /**
    *  Allows the user to browse for one or more signature files.
    *
    *  This method invokes a file selection dialog in which the user can select one or more
    *  signature files.  The files are then added to the list in the import extension widget
    *  of the dialog.  This method is called automatically when the user clicks the Browse
    *  button on the import extension widget.
    */
   void browseFiles();

   /**
    *  Invokes the search dialog for user to search the disk for signatures.
    *
    *  This method invokes a search dialog in which the user can search multiple directories
    *  for signature files to import.  If the user accepts the search, all signature files
    *  found during the search are added to the list in the import extension widget of the
    *  dialog.  This method is called automatically when the user clicks the Search button on
    *  the import extension widget.
    */
   void searchDirectories();

   /**
    *  Loads selected signatures in the import extension widget.
    *
    *  This method loads signatures from selected signature files in the import extension
    *  widget of the dialog.  The main signature list view widget is updated if necessary.
    *  This method is called automatically when the user clicks the Load button on the import
    *  extension widget.
    */
   void loadSignatures();

   /**
    *  Updates the main list view with the available signatures.
    *
    *  This method updates the signature items in the list view based on the selected display
    *  filter type.
    */
   virtual void updateSignatureList();

   /**
    *  Performs tasks associated with the custom button.
    *
    *  This slot method displays a QMessageBox stating that the actions for the custom
    *  button have not been set up. Classes derived from SignatureSelector need to overload
    *  this method with the actions for their custom button. 
    */
   virtual void customButtonClicked();

private:
   Progress* mpProgress;
   Service<PlugInManagerServices> mpManager;
   Service<ModelServices> mpModel;
   Service<ObjectFactory> mpObjFact;
   Service<UtilityServices> mpUtilities;

   QComboBox* mpFormatCombo;
   QTreeWidget* mpSignatureList;
   QGridLayout* mpEmptyLayout;
   QPushButton* mpPropertiesButton;
   QPushButton* mpExportButton;
   QPushButton* mpUnloadButton;
   QPushButton* mpImportButton;
   QPushButton* mpApplyButton;
   QPushButton* mpCustomButton;
   QWidget* mpImportWidget;
   QListWidget* mpFilesList;

   QStringList mImporterFilters;
   std::map<QTreeWidgetItem*, Signature*> mLoadedSignatures;
   SearchDlg* mpSearchDlg;

private:
   QTreeWidgetItem* addSignatureItem(Signature* pSignature, QTreeWidgetItem* pParentItem = NULL);
   bool searchForMetadata(Signature* pSignature, const QString& strMetadataName,
      const QString& strMetadataValue);
};

#endif
