/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SECURITYMARKINGSDLG_H
#define SECURITYMARKINGSDLG_H

#include <QtGui/QComboBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>

#include <string>
#include <vector>

#include "DynamicObject.h"
#include "DataVariant.h"
#include "ConfigurationSettings.h"


class Classification;
class DateTime;
class QPushButton;

/**
 *  A dialog to display security markings.
 *
 *  The security markings dialog provides the means by which users can view and/or edit
 *  security markings.  The dialog can be initialized from a Classification object or
 *  a string containing the markings.  If no initialization object is provided, the dialog
 *  can be used to obtain arbitrary markings from the user.  If a classification object
 *  is used for initialization of the dialog, any user edits will also update the values
 *  in the classification object.
 *
 *  The dialog contains two tabs: one tab for the markings that are displayed at the top
 *  and bottom of an object or product, and another tab for ancillary information with the
 *  classification.
 *
 *  The first tab allows users to choose the overall classification level and the values
 *  of the various classification fields that also comprise the overall markings string.
 *  A preview of the final classification string is provided.
 *
 *  The second tab contains the following information that is included in the classification
 *  object:
 *  - Classification reason
 *  - Declassification type
 *  - Declassification date
 *  - File downgrade
 *  - Downgrade date
 *  - File control
 *  - Description
 *  - Copy number
 *  - Number of copies
 *
 *  At the bottom of the dialog outside of the tab widget is the favorites list.  This list
 *  allows users to store commonly used classification markings for the purpose of easily
 *  updating the markings for multiple objects in multiple instances of the dialog.  The
 *  favorites list is stored in the user's configuration file when the user accepts changes
 *  in the dialog.
 *
 *  @see        Classification
 */
class SecurityMarkingsDlg : public QDialog
{
   Q_OBJECT

public:
   SETTING_PTR(Favorites, SecurityMarkings, DynamicObject)

   /**
    *  Creates and populates the security markings dialog.
    *
    *  The constructor creates the widgets and initializes the values if a Classification
    *  object or string is given.  If both a Classification object and string are given,
    *  the dialog values are populated using the Classification object first, and then the
    *  string is used to populate the overall markings string.
    *
    *  @param   parent
    *           The parent widget.
    *  @param   strInitialMarkings
    *           The optional markings string that is used to initialize the markings.
    *  @param   pClassification
    *           The optional classification object to display and edit.
    */
   SecurityMarkingsDlg::SecurityMarkingsDlg(QWidget* parent = 0, const QString& strInitialMarkings = QString(),
      Classification* pClassification = NULL);

   /**
    *  Destroys the dialog.
    */
   ~SecurityMarkingsDlg();

   /**
    *  Returns the current security markings.
    *
    *  @return  The classification markings string in the same format as returned by
    *           Classification::getClassificationText().
    */
   QString getSecurityMarkings() const;

protected:
   /**
    *  Populates the list of available markings for a given dialog widget.
    *
    *  This method populates one or all widgets on the first tab of the dialog
    *  displaying the classification markings.  The list values are populated
    *  according to the values contained in the support files found in the Security
    *  Markings directory specified in the %Options dialog (Support Files Directory).
    *  The method also populates the favorites combo box from the user configuration settings.
    *
    *  @param   pWidget
    *           The widget to initialize from the support files, or NULL to
    *           initialize all widgets.
    */
   void initialize(QWidget* pWidget = NULL);

   /**
    *  Sets the values of the dialog widgets to the given classification object values.
    *
    *  @param   pClass
    *           The classification object from which to initialize the dialog widget
    *           values.  This method does nothing if NULL is passed in.
    */
   void importFromClassification(Classification *pClass);

   /**
    *  Updates the values in a given classification object to the current values in
    *  the dialog widgets.
    *
    *  @param   pClass
    *           The classification object to update from the dialog widget values.
    *           This method does nothing if NULL is passed in.
    */
   void exportToClassification(Classification *pClass);

   /**
    *  Writes the favorites list to the user configuration settings and serializes
    *  the configuration settings to the configuration file.
    *
    *  @return  Returns true if the favorites list was successfully written to the
    *           user configuration settings, otherwise returns false.
    */
   bool serializeFavorites();

   /**
    *  Reads the favorites list from the user configuration settings and populates
    *  the favorites combo box.
    *
    *  @return  Returns true if the favorites list was successfully populated from
    *           the user configuration settings, otherwise returns false.
    */
   bool deserializeFavorites();

   /**
    *  Populates the favorties combo box from the current list of favorites.
    */
   void updateFavoritesCombo();

   /**
    *  Clears the favorites list.
    *
    *  This method clears the favorites list but does not change the contents of the
    *  favorites combo box.  To update the favorites combo box, call
    *  updateFavoritesCombo().
    */
   void clearFavorites();

protected slots:
   /**
    *  Adds a new entry to the current classification component list.
    *
    *  This method prompts the user to specify the name for a new entry in the currently
    *  selected classification component list (Codewords, System, Releasability, and
    *  Exemption).
    */
   void addListItem();

   /**
    *  Restores the current classification component list to its initial entries.
    *
    *  This method updates the entries in the currently selected classification component
    *  list (Codewords, System, Releasability, and Exemption) to the values contained in
    *  the support files located in the Security Markings directory specified in the
    *  %Options.
    */
   void resetList();

   /**
    *  Updates the markings preview widget.
    *
    *  This method updates the markings string in the preview widget of the dialog to
    *  reflect the current widget values.  This method is called automatically when the
    *  user changes widget values and should not need to be called directly.
    */
   void updateMarkings();

   /**
    *  Updates the dialog size based on the currently selected tab in the tab widget.
    *
    *  This method is called automatically when the user changes the active tab and should
    *  not need to be called directly.
    */
   void updateSize();

   /**
    *  Updates the classification object and user configuration file.
    *
    *  This method updates the classification object passed into the constructor with the
    *  current widget values and serializes the favorites list in the user configuration
    *  file.  The method is called automatically when the user clicks the OK button in the
    *  dialog and should not need to be called directly.
    */
   void accept();

   /**
    *  Adds a new item to the favorites list.
    *
    *  This method adds the security markings specified by the current widget values to the
    *  favorites list and updates the favorites combo box.
    */
   void addFavoriteItem();

   /**
    *  Removes the currently selected favorites item from the favorites list.
    *
    *  This method removes the current favorites item specified in the favorites combo box.
    */
   void removeFavoriteItem();

   /**
    *  Sets the widgets values to the currently selected favorites item.
    *
    *  This method updates the widgets and the markings preview to the values specified by
    *  the currently selected item in the favorites combo box.
    */
   void favoriteSelected();

private:
   QStackedWidget* mpValueStack;
   QTabWidget* mpTabWidget;
   QComboBox* mpClassLevelCombo;
   QListWidget* mpCodewordList;
   QListWidget* mpSystemList;
   QListWidget* mpCountryCodeList;
   QListWidget* mpFileReleasingList;
   QListWidget* mpExemptionList;
   QComboBox* mpClassReasonCombo;
   QComboBox* mpDeclassTypeCombo;
   QDateEdit* mpDeclassDateEdit;
   QComboBox* mpFileDowngradeCombo;
   QDateEdit* mpDowngradeDateEdit;
   QComboBox* mpFileControlCombo;
   QTextEdit* mpDescriptionEdit;
   QPushButton* mpResetDowngradeDateButton;
   QPushButton* mpResetDeclassDateButton;
   QSpinBox* mpCopyNumberSpinBox;
   QSpinBox* mpNumberOfCopiesSpinBox;
   QTextEdit* mpMarkingsEdit;
   QPushButton *mpAddButton;

   QComboBox *mpFavoritesCombo;

   Classification* mpClass;

   std::vector<Classification*> mlstFavorites;
   std::vector<std::string> mlstRemovedFavorites;

   bool mDeclassDateIsValid;
   bool mDowngradeDateIsValid;

private:
   QString getListString(QListWidget* pListWidget, const QString& strDelimiter);
   void selectListFromString(QListWidget* pListWidget, const QString& strText);
   void selectComboFromString(QComboBox* pComboBox, const QString& strText);
   void setDateEdit(QDateEdit* pDateEdit, const DateTime* pDateTime);
   void dateChanged(const QObject *pSender, const QDate &date);
   void loadCountryCodes();

private slots:
   void checkAddButton(int iIndex);
   void resetDateTimeEdit();
   void dateChanged(const QDate &date);
};

#endif
