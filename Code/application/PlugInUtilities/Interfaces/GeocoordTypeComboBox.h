/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOCOORDTYPECOMBOBOX_H
#define GEOCOORDTYPECOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

/**
 *  A widget to choose a geocoordinate type from a non-editable combo box.
 *
 *  @see        GeocoordType
 */
class GeocoordTypeComboBox : public QComboBox
{
   Q_OBJECT

public:
   /**
    *  Creates a geocoordinate type combo box.
    *
    *  Items for each \link ::GeocoordType GeocoordType\endlink value are
    *  automatically added, and the combo box is set to not allow editing.
    *
    *  @param   pParent
    *           The parent widget, which is passed into the QComboBox
    *           constructor.
    */
   GeocoordTypeComboBox(QWidget* pParent);

   /**
    *  Destroys the geocoordinate type combo box.
    */
   virtual ~GeocoordTypeComboBox();

   /**
    *  Sets the current geocoordinate type.
    *
    *  @param   geocoordType
    *           The geocoordinate type to set as the current item in the combo
    *           box.  If \em geocoordType is invalid, the combo box is set to
    *           not have a selected item (i.e. QComboBox::currentIndex() returns
    *           -1).
    */
   void setGeocoordType(GeocoordType geocoordType);

   /**
    *  Returns the current geocoordinate type.
    *
    *  @return  Returns the current geocoordinate type.  The returned value is
    *           invalid if no geocoordinate type is selected.
    */
   GeocoordType getGeocoordType() const;

signals:
   /**
    *  Emitted when the user changes the current geocoordinate type in the
    *  combo box.
    *
    *  @param   geocoordType
    *           The newly selected geocoordinate type.
    */
   void geocoordTypeChanged(GeocoordType geocoordType);

private:
   GeocoordTypeComboBox(const GeocoordTypeComboBox& rhs);
   GeocoordTypeComboBox& operator=(const GeocoordTypeComboBox& rhs);

private slots:
   void translateIndexChanged(const QString& text);
};

#endif
