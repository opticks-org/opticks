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

#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include "ModelServices.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"

#include <string>
#include <vector>

class DynamicObject;
class Progress;
class SearchDlg;
class Signature;
class SignatureSet;

/**
 *  A dialog to import and select signatures.
 *
 *  The signature selector is a dialog that allows users to select one or more
 *  signatures from a list.  The dialog displays a list of signatures containing
 *  all Signature objects currently loaded into the data model. The user has the
 *  option of creating one or more filters that will reduce the number of
 *  signatures displayed in the list.  Filters can be created based on any
 *  combination of the following signature attributes:
 *  - %Signature Name
 *  - Metadata Attribute Name
 *  - Metadata Attribute Value
 *
 *  When creating the filter, wildcarding and case sensitivity options can be
 *  set separately on each of the filter criteria listed above.  The filter can
 *  also be applied to signature libraries as a whole, or to the individual
 *  signatures contained within the library.
 *
 *  If the user does not find the desired signature(s) in the list, an Import
 *  button is provided for the user to import one or more signatures while the
 *  dialog remains active.
 *
 *  When the user clicks the Import button, the dialog is expanded to contain a
 *  separate list of signature files from which the user can select one or more
 *  signatures to import.  After import, the main signature list is updated
 *  appropriately.
 *
 *  On signature import, the user can browse the disk for one or more signature
 *  files in a single directory or the user can search one or more directories
 *  for recognized signature files.
 *
 *  When a single signature in the main signature list is selected, the user can
 *  view the signature's properties by clicking on the %Properties button in the
 *  dialog.
 *
 *  By default, the dialog contains OK and Cancel buttons.  For modeless
 *  instantiations of the dialog an Apply button can be added by setting the
 *  appropriate parameter in the constructor.  When the dialog contains an Apply
 *  button, it can be enabled and disabled by calling enableApplyButton().  A
 *  custom button can also be added to the dialog by passing the button text
 *  into the constructor.  When the dialog contains a custom button, it can be
 *  enabled and displayed by calling enableCustomButton().
 *
 *  @see        Signature, SignatureSet
 */
class SignatureSelector : public QDialog
{
   Q_OBJECT

public:
   /**
    *  Creates the signature selector dialog.
    *
    *  The constructor creates the widgets and initializes the signature list
    *  based on all loaded signatures in the data model.
    *
    *  @param   pProgress
    *           An optional Progress object that is used when searching for
    *           signatures to import.
    *  @param   pParent
    *           The dialog's parent widget.
    *  @param   mode
    *           The selection mode to be used for the list of signatures.
    *  @param   addApply
    *           If \c true, an Apply button will appear.
    *  @param   customButtonLabel
    *           Label that will appear on the custom button.  The custom button
    *           will only be added to the dialog if \c customButtonLabel is not
    *           empty.
    */
   SignatureSelector(Progress* pProgress, QWidget* pParent = NULL,
      QAbstractItemView::SelectionMode mode = QAbstractItemView::ExtendedSelection, bool addApply = false,
      const std::string& customButtonLabel = std::string());

   /**
    *  Destroys the signature selector dialog.
    */
   ~SignatureSelector();

   /**
    *  Returns a vector of currently selected signatures.
    *
    *  This method returns a vector of all selected signatures in the list view.
    *  If a signature set is selected, only the SignatureSet object is added to
    *  the vector.  %Any selected signatures contained in the selected signature
    *  set are not added to the vector.
    *
    *  To obtain a vector that includes Signature objects inside a selected
    *  SignatureSet object, call getExtractedSignatures() instead.
    *
    *  @return  A vector of the selected Signature objects.
    */
   virtual std::vector<Signature*> getSignatures() const;

   /**
    *  Returns a vector of currently selected signatures.
    *
    *  This method returns a vector of each individual signature based on the
    *  selected items in the list view.  If a signature set is selected, the
    *  SignatureSet object is not added to the vector, but each individual
    *  signature contained in the SignatureSet object is added.  Signatures
    *  contained inside a selected signature set are only added to the vector
    *  once, regardless of whether or not the signature inside the selected
    *  signature set is also selected.  If signatures are filtered out of a
    *  selected signature set object, only the signatures in the set that pass
    *  the filter are added to the vector.
    *
    *  To obtain a vector of just the selected signature and signature set
    *  objects, call getSignatures() instead.
    *
    *  @return  A vector containing the individual Signature objects based on
    *           the selected items in the list view.
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
    *  Adds a top-level item to the signature display format tree widget.
    *
    *  This is used by subclasses to add custom signature types. In addition to
    *  calling this method, a subclass will usually need to implement
    *  updateSignatureList() to populate the signature list when the custom type
    *  is selected. Also, getSignatures() will need to be implemented to return
    *  the selected signature when the custom type is selected.
    *
    *  @param type
    *         The type name to add.
    */
   void addCustomType(const QString& type);

   /**
    *  Access the current signature format type.
    *
    *  @return The current signature format type.
    */
   QString getCurrentFormatType() const;

   /**
    *  Access the signature list.
    *
    *  Used to add items to the signature list when a custom format type is selected.
    *
    *  @return The signature list.
    */
   QTreeWidget* getSignatureList() const;

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
    *  Returns the number of selected items in the signature list.
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
    *  This method enables or disables the Apply button if the dialog was
    *  set to contain an Apply button in the constructor.  When the dialog is
    *  created with an Apply button, it is disabled by default.  This method
    *  does nothing if the dialog is created without an Apply button.
    *
    *  @param   enable
    *           Set this parameter to \c true to enable the Apply button or to
    *           \c false to disable the Apply button.
    *
    *  @see     isApplyButtonEnabled()
    */
   void enableApplyButton(bool enable);

   /**
    *  Enables or disables the custom button.
    *
    *  This method enables or disables the custom button if the dialog was
    *  set to contain a custom button in the constructor.  When the dialog is
    *  created with a custom button, it is enabled by default.  This method does
    *  nothing if the dialog is created without a custom button.
    *
    *  @param   enable
    *           Set this parameter to \c true to enable the custom button or to
    *           \c false to disable the custom button.
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
    *  Sets the format type that is used to display signatures in the list.
    *
    *  This method updates the signature list to display different sets of
    *  signatures.  By default, "Signatures" and the names of any created
    *  filters are available format types.  Custom format types are also
    *  available if they have been added by calling addCustomType().
    *
    *  @warning To maintain compatibility with earlier versions, the following
    *           strings are special types that perform special behavior.
    *           Support for these strings may be removed in a future release, so
    *           they should not be used in any new code.
    *           - "Metadata..." - Invokes a dialog to create a new filter.  This
    *             is identical to clicking on the Create Filter button.
    *           - "-----------------------" - Sets the current display type to
    *             the top-level "Signatures" item.  This is identical to
    *             selecting the "Signatures" item directly.
    *
    *  @param   strFormat
    *           The display format type for the signature list.
    */
   void setDisplayType(const QString& strFormat);

   /**
    *  Enables or disables the %Properties, Export, and Delete buttons.
    *
    *  This method enables or disables the %Properties, Export, and Delete
    *  buttons based on the number of selected signature items in the list.
    */
   void enableButtons();

   /**
    *  Invokes a properties dialog for the currently selected signature.
    *
    *  This method invokes an instance of a SignaturePropertiesDlg for the first selected
    *  signature item in the list view.  This method is called automatically when the user
    *  clicks the %Properties button.
    */
   void displaySignatureProperties();

   /**
    *  Export currently selected signatures as a signature set.
    */
   void exportSignatures();

   /**
    *  Destroys currently selected signatures.
    *
    *  This method destroys Signature objects in the data model from all
    *  currently selected signature items in the list view.  This method is
    *  called automatically when the user clicks the Delete button.
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
    *  button have not been set up. Classes derived from SignatureSelector need to override
    *  this method with the actions for their custom button.
    */
   virtual void customButtonClicked();

private:
   SignatureSelector(const SignatureSelector& rhs);
   SignatureSelector& operator=(const SignatureSelector& rhs);

   Progress* mpProgress;
   Service<PlugInManagerServices> mpManager;
   Service<ModelServices> mpModel;

   static const QString sMetadataType;
   static const QString sDashType;

   QTreeWidget* mpFormatTree;
   QTreeWidgetItem* mpSignaturesItem;
   QTreeWidget* mpSignatureList;
   QAction* mpCreateFilterAction;
   QAction* mpEditFilterAction;
   QAction* mpDeleteFilterAction;
   QAction* mpPropertiesAction;
   QAction* mpExportAction;
   QAction* mpDeleteAction;
   QGridLayout* mpEmptyLayout;
   QPushButton* mpImportButton;
   QPushButton* mpApplyButton;
   QPushButton* mpCustomButton;
   QWidget* mpImportWidget;
   QListWidget* mpFilesList;

   QStringList mImporterFilters;
   SearchDlg* mpSearchDlg;

   void addSignature(Signature* pSignature);
   QTreeWidgetItem* createSignatureItem(Signature* pSignature, QTreeWidgetItem* pParentItem = NULL,
      bool createLibraryItems = false);
   void createLibraryItems(SignatureSet* pSignatureSet, QTreeWidgetItem* pParentItem,
      const QRegExp& signatureNameFilter, const QRegExp& metadataNameFilter, const QRegExp& metadataValueFilter);
   bool matchLibrarySignatures(const SignatureSet* pSignatureSet, const QRegExp& signatureNameFilter,
      const QRegExp& metadataNameFilter, const QRegExp& metadataValueFilter);
   bool matchSignature(const Signature* pSignature, const QRegExp& signatureNameFilter,
      const QRegExp& metadataNameFilter, const QRegExp& metadataValueFilter);
   bool matchMetadata(const DynamicObject* pMetadata, const QRegExp& nameFilter, const QRegExp& valueFilter);
   void extractFromSigSets(const std::vector<Signature*>& sourceSigs, std::vector<Signature*>& destSigs) const;
   bool isFilterNameUnique(const QString& filterName, QTreeWidgetItem* pIgnoreItem = NULL) const;

private slots:
   void createFilter();
   void editFilter();
   void checkFilterName(QTreeWidgetItem* pItem, int column);
   void deleteFilter();
   void formatChanged();
   void displayFormatContextMenu(const QPoint& menuPoint);
   void displaySignatureContextMenu(const QPoint& menuPoint);
};

#endif
