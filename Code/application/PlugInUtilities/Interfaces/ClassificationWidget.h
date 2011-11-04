/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CLASSIFICATIONWIDGET_H
#define CLASSIFICATIONWIDGET_H

#include "DynamicObject.h"
#include "ConfigurationSettings.h"

#include <QtGui/QComboBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QListWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

#include <string>
#include <vector>

class Classification;
class DateTime;
class QPushButton;

/**
 *  A widget to display classification markings.
 *
 *  The classification widget provides the means by which users can view and/or
 *  edit classification markings.  The widget contains two tabs: one tab for the
 *  markings that are displayed at the top and bottom of an object or product,
 *  and another tab for ancillary information with the classification.
 *
 *  The first tab allows users to choose the overall classification level and
 *  the values of the various classification fields that also comprise the
 *  overall markings string.  A preview of the final classification string is
 *  provided.
 *
 *  The second tab contains the following information that is included in the
 *  classification object:
 *  - %Classification reason
 *  - Declassification type
 *  - Declassification date
 *  - File downgrade
 *  - Downgrade date
 *  - File control
 *  - Description
 *  - Copy number
 *  - Number of copies
 *
 *  At the bottom of the widget outside of the tab widget is the favorites list.
 *  This list allows users to store commonly used classification markings for
 *  the purpose of easily updating the markings for multiple objects.  The
 *  favorites list is stored in the user's configuration file when the changes
 *  in the widget are applied back to the Classification object.
 *
 *  @see        Classification
 */
class ClassificationWidget : public QWidget
{
   Q_OBJECT

public:
   SETTING_PTR(Favorites, SecurityMarkings, DynamicObject)

   /**
    *  Creates the classification widget.
    *
    *  @param   pParent
    *           The parent widget.
    */
   ClassificationWidget(QWidget* pParent = NULL);

   /**
    *  Destroys the classification widget.
    */
   virtual ~ClassificationWidget();

   /**
    *  Sets the widget to view and edit the markings of a given Classification
    *  object.
    *
    *  This method sets the internal Classification object that is being edited
    *  to the given classification.  The modified flag is reset to \c false, and
    *  any previous changes that have not been applied back to the previous
    *  Classification object are discarded.
    *
    *  @param   pClassification
    *           The Classification object to display and edit.
    *  @param   initializeWidgets
    *           If set to \c true, the widgets are initialized to the values
    *           contained in the given Classification object.  If set to
    *           \c false the widgets are initialized to empty values.
    *
    *  @see     isModified(), applyChanges()
    */
   void setClassification(Classification* pClassification, bool initializeWidgets = true);

   /**
    *  Queries whether changes have been made to the classification markings.
    *
    *  This method queries whether changes have been made since previous changes
    *  were last applied, or since the Classification object was originally set
    *  (i.e. the last call to setClassification()).
    *
    *  @return  Returns \c true if the user has changed the classification
    *           markings; otherwise returns \c false.
    */
   bool isModified() const;

   /**
    *  Applies changes back to the Classification object.
    *
    *  This method also saves the current state of markings designated as
    *  favorites if changes are successfully applied back to the Classification
    *  object.
    *
    *  @return  Returns \c true if the changes were successfully applied back to
    *           the Classification object or if no changes have been made.
    *           Returns \c false if no Classification object has been set or if
    *           the user has not selected a valid classification level.
    */
   bool applyChanges();

signals:
   /**
    *  Emitted when the user changes any classification marking.
    *
    *  @param   modified
    *           This parameter is provided as a convenience to connect to slots
    *           that have a \c bool input parameter  The signal is always
    *           emitted with a value of \c true.
    */
   void modified(bool modified = true);

protected:
   /**
    *  Populates the list of available markings for a given child widget.
    *
    *  This method populates one or all widgets on the first tab displaying the
    *  classification markings.  The list values are populated according to the
    *  values contained in the support files found in the Security Markings
    *  directory specified in the %Options dialog (Support Files Directory).
    *  The method also populates the favorites combo box from the user
    *  configuration settings.
    *
    *  @param   pWidget
    *           The widget to initialize from the support files, or \c NULL to
    *           initialize all widgets.
    */
   void initialize(QWidget* pWidget = NULL);

   /**
    *  Sets the values of the widgets to the given Classification object values.
    *
    *  @param   pClass
    *           The Classification object from which to initialize the widget
    *           values.  This method does nothing if \c NULL is passed in.
    */
   void importFromClassification(const Classification* pClass);

   /**
    *  Updates the values in a given Classification object to the current widget
    *  values.
    *
    *  @param   pClass
    *           The Classification object to update from the widget values.
    *           This method does nothing if \c NULL is passed in.
    */
   void exportToClassification(Classification* pClass);

   /**
    *  Writes the favorites list to the user configuration settings and
    *  serializes the configuration settings to the configuration file.
    *
    *  @return  Returns \c true if the favorites list was successfully
    *           written to the user configuration settings; otherwise
    *           returns \c false.
    */
   bool serializeFavorites();

   /**
    *  Reads the favorites list from the user configuration settings and
    *  populates the favorites combo box.
    *
    *  @return  Returns \c true if the favorites list was successfully
    *           populated from the user configuration settings; otherwise
    *           returns \c false.
    */
   bool deserializeFavorites();

   /**
    *  Populates the favorites combo box from the current list of favorites.
    */
   void updateFavoritesCombo();

   /**
    *  Clears the favorites list.
    *
    *  This method clears the favorites list but does not change the contents
    *  of the favorites combo box.  To update the favorites combo box, call
    *  updateFavoritesCombo().
    */
   void clearFavorites();

protected slots:
   /**
    *  Adds a new entry to the current classification component list.
    *
    *  This method prompts the user to specify the name for a new entry in the
    *  currently selected classification component list (Codewords, System,
    *  Releasability, and Exemption).
    */
   void addListItem();

   /**
    *  Restores the current classification component list to its initial
    *  entries.
    *
    *  This method updates the entries in the currently selected classification
    *  component list (Codewords, System, Releasability, and Exemption) to the
    *  values contained in the support files located in the Security Markings
    *  directory specified in the %Options.
    */
   void resetList();

   /**
    *  Updates the markings preview widget.
    *
    *  This method updates the markings string in the preview widget to reflect
    *  the current widget values.  This method is called automatically when the
    *  user changes widget values and should not need to be called directly.
    */
   void updateMarkings();

   /**
    *  Updates the widget size based on the currently selected tab in the tab
    *  widget.
    *
    *  This method is called automatically when the user changes the active tab
    *  and should not need to be called directly.
    */
   void updateSize();

   /**
    *  Adds a new item to the favorites list.
    *
    *  This method adds the security markings specified by the current widget
    *  values to the favorites list and updates the favorites combo box.
    */
   void addFavoriteItem();

   /**
    *  Removes the currently selected favorites item from the favorites list.
    *
    *  This method removes the current favorites item specified in the
    *  favorites combo box.
    */
   void removeFavoriteItem();

   /**
    *  Sets the widgets values to the currently selected favorites item.
    *
    *  This method updates the widgets and the markings preview to the values
    *  specified by the currently selected item in the favorites combo box.
    */
   void favoriteSelected();

private:
   ClassificationWidget(const ClassificationWidget& rhs);
   ClassificationWidget& operator=(const ClassificationWidget& rhs);

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
   QPushButton* mpAddButton;

   QComboBox* mpFavoritesCombo;

   Classification* mpClass;
   bool mModified;

   std::vector<Classification*> mlstFavorites;
   std::string mFavoritesAttributePrefix;

   bool mDeclassDateIsValid;
   bool mDowngradeDateIsValid;

private:
   QString getListString(QListWidget* pListWidget, const QString& strDelimiter);
   void selectListFromString(QListWidget* pListWidget, const QString& strText);
   void selectComboFromString(QComboBox* pComboBox, const QString& strText);
   void setDateEdit(QDateEdit* pDateEdit, const DateTime* pDateTime);
   void dateChanged(const QObject *pSender, const QDate &date);

private slots:
   void checkAddButton(int iIndex);
   void resetDateTimeEdit();
   void dateChanged(const QDate &date);
};

#endif
