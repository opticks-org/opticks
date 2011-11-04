/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef LATLONLINEEDIT_H
#define LATLONLINEEDIT_H

#include <QtGui/QLineEdit>

#include "GeoPoint.h"

/**
 *  A line edit for a latitude or longitude value.
 *
 *  The LatLonLineEdit provides a line edit where user can enter a latitude or longitude
 *  value.  The line edit effectively wraps a DmsPoint value, where after entering a value,
 *  the format of the text changes to the format specified by the current DmsPoint::DmsType
 *  value.
 *
 *  @see        DmsPoint
 */
class LatLonLineEdit : public QLineEdit
{
   Q_OBJECT

public:
   /**
    *  Creates a new line edit.
    *
    *  The line edit is created with a default display type of DmsPoint::DmsType::DMS_DECIMAL.
    *
    *  @param   parent
    *           The parent widget.
    */
   LatLonLineEdit(QWidget* parent = 0);

   /**
    *  Creates a new line edit.
    *
    *  The line edit is created and initialized with the given value.  The initial display
    *  format is set to the DmsPoint::DmsType value in the given DmsPoint value.
    *
    *  @param   dmsPoint
    *           The initial latitude or longitude value.
    *  @param   parent
    *           The parent widget.
    */
   LatLonLineEdit(DmsPoint dmsPoint, QWidget* parent = 0);

   /**
    *  Destroys the line edit.
    */
   ~LatLonLineEdit();

   /**
    *  Sets the current latitude or longitude value.
    *
    *  @param   dmsPoint
    *           The new current value.  The display type is updated to the DmsPoint::DmsType
    *           value in the given DmsPoint value.
    *
    *  @see     getMinimumValue(), getMaximumValue()
    */
   void setValue(DmsPoint dmsPoint);

   /**
    *  Returns the current value represented by the text in the line edit.
    *
    *  @return  The current value, which is equivalent to DmsPoint::getValue().  A value of
    *           0.0 is returned if the current line edit text is empty.
    *
    *  @see     setValue(), DmsPoint
    */
   double getValue() const;

   /**
    *  Sets the line edit to automatically update the text format.
    *
    *  This method sets an internal flag to automatically update the text when the keyboard
    *  focus leaves the line edit widget.  The text is updated to the format specified by
    *  DmsPoint::DmsType.
    *
    *  @param   bAutoUpdate
    *           Set this value to true to automatically update the format of the text when
    *           the keyboard focus leaves the widget.  Set this value to false to not
    *           automatically update the format of the text.
    *
    *  @see     hasAutoUpdate()
    */
   void setAutoUpdate(bool bAutoUpdate);

   /**
    *  Queries whether the line edit automatically updates the text format.
    *
    *  @return  True if the line edit automatically updates the format of the text when the
    *           keyboard focus leaves the widget, otherwise false.
    *
    *  @see     setAutoUpdate()
    */
   bool hasAutoUpdate() const;

protected:
   /**
    *  Updates the format of the current text.
    *
    *  If the auto update flag is set to true, this method updates the text format to the
    *  format specified by the current DmsPoint::DmsType value.
    *
    *  @param   pEvent
    *           The focus event causing the update.
    *
    *  @see     hasAutoUpdate()
    */
   void focusOutEvent(QFocusEvent* pEvent);

private:
   LatLonLineEdit(const LatLonLineEdit& rhs);
   LatLonLineEdit& operator=(const LatLonLineEdit& rhs);
   bool mAutoUpdate;
   DmsPoint::DmsType mDisplayType;
};

#endif
