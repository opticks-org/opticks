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

#include "Classification.h"
#include "ConfigurationSettings.h"
#include "DynamicObject.h"
#include "SafePtr.h"

#include <QtGui/QWidget>

#include <boost/any.hpp>
#include <string>
#include <vector>

class DateTime;
class QComboBox;
class QDate;
class QDateEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTabWidget;
class QTextEdit;
class Subject;

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
 *  favorites list is stored in the user's configuration file as the user adds
 *  or removes items from the list.
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
    *  to the given classification.  All changes to the widgets are applied
    *  immediately to the Classification object.
    *
    *  @param   pClassification
    *           The Classification object to display and edit.
    *  @param   initializeWidgets
    *           If set to \c true, the widgets are initialized to the values
    *           contained in the given Classification object.  If set to
    *           \c false the widgets are initialized to empty values.
    */
   void setClassification(Classification* pClassification, bool initializeWidgets = true);

protected:
   /**
    *  Initializes the widgets if the Classification object has been modified
    *  since the widget was last displayed.
    *
    *  This method is used internally and should not be called directly.
    *
    *  @param   pEvent
    *           The show event causing the update.
    */
   virtual void showEvent(QShowEvent* pEvent);

   /**
    *  Attaches to the Classification object to determine whether the widgets
    *  should be initialized when the widget is shown.
    *
    *  This method is used internally and should not be called directly.
    *
    *  @param   pEvent
    *           The hide event causing the update.
    */
   virtual void hideEvent(QHideEvent* pEvent);

   /**
    *  Writes the favorites list to the user configuration settings and
    *  serializes the configuration settings to the configuration file.
    *
    *  This method is called automatically when the user adds or removes items
    *  from the favorites list.
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
    *  This method is called automatically when the widget is created.
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
    *  Updates the markings preview widget.
    *
    *  This method updates the markings string in the preview widget to reflect
    *  the current widget values.  This method is called automatically when the
    *  user changes widget values and should not need to be called directly.
    */
   void updateMarkings();

private:
   ClassificationWidget(const ClassificationWidget& rhs);
   ClassificationWidget& operator=(const ClassificationWidget& rhs);

   QComboBox* mpClassLevelCombo;
   QTabWidget* mpTabWidget;
   QStackedWidget* mpValueStack;
   QListWidget* mpCodewordList;
   QListWidget* mpSystemList;
   QListWidget* mpFileReleasingList;
   QListWidget* mpCountryCodeList;
   QListWidget* mpExemptionList;
   QPushButton* mpAddButton;
   QComboBox* mpClassReasonCombo;
   QComboBox* mpDeclassTypeCombo;
   QDateEdit* mpDeclassDateEdit;
   QPushButton* mpResetDeclassDateButton;
   QComboBox* mpFileDowngradeCombo;
   QDateEdit* mpDowngradeDateEdit;
   QPushButton* mpResetDowngradeDateButton;
   QComboBox* mpFileControlCombo;
   QTextEdit* mpDescriptionEdit;
   QSpinBox* mpCopyNumberSpinBox;
   QSpinBox* mpNumberOfCopiesSpinBox;
   QTextEdit* mpMarkingsEdit;
   QComboBox* mpFavoritesCombo;

   SafePtr<Classification> mpClass;
   std::vector<Classification*> mlstFavorites;
   bool mNeedsInitialization;

private:
   void resetToDefault(QWidget* pWidget = NULL);
   void initialize();

   QString getListString(QListWidget* pListWidget);
   void selectListFromString(QListWidget* pListWidget, const QString& strText);
   void selectComboFromString(QComboBox* pComboBox, const QString& strText);
   void setDateEdit(QDateEdit* pDateEdit, const DateTime* pDateTime);

   void classificationModified(Subject& subject, const std::string& signal, const boost::any& value);

private slots:
   void setLevel(const QString& level);
   void updateSize();
   void checkAddButton(int iIndex);
   void setCodewords();
   void setSystem();
   void setFileReleasing();
   void setCountryCode();
   void setDeclassificationExemption();
   void addListItem();
   void resetList();
   void setClassificationReason(const QString& reason);
   void setDeclassificationType(const QString& declassType);
   void setDeclassificationDate(const QDate& declassDate);
   void resetDeclassificationDate();
   void setFileDowngrade(const QString& fileDowngrade);
   void setDowngradeDate(const QDate& downgradeDate);
   void resetDowngradeDate();
   void setFileControl(const QString& fileControl);
   void setDescription();
   void setFileCopyNumber(const QString& copyNumber);
   void setFileNumberOfCopies(const QString& numberOfCopies);
   void favoriteSelected();
   void addFavoriteItem();
   void removeFavoriteItem();
};

#endif
