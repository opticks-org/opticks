/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTERPOLATIONCOMBOBOX_H
#define INTERPOLATIONCOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

/**
 *  A combo box that will allow a user to select an interpolation method.
 *
 *  This is a read-only combo box that provides selection values based on the
 *  ::InterpolationType enum values.
 */
class InterpolationComboBox : public QComboBox
{
   Q_OBJECT

public:
   /**
    *  Creates a new interpolation combo box.
    *
    *  @param   pParent
    *           The parent widget.
    */
   InterpolationComboBox(QWidget* pParent = NULL);

   /**
    *  Destroys the combo box.
    */
   virtual ~InterpolationComboBox();

   /**
    *  Set the interpolation method value.
    *
    * @param   interp
    *          The interpolation method to display.
    */
   void setInterpolation(InterpolationType interp);

   /**
    *  Get the interpolation method value.
    *
    * @return   The current interpolation method displayed.
    */
   InterpolationType getInterpolation() const;

signals:
   /**
    *  Emitted when the user selects a interpolation method in the combo box.
    *
    *  This signal is not emitted if the change is initiated by a setInterpolation()
    *  call.
    *
    *  @param   interp
    *           The newly selected interpolation method in the combo box.
    */
   void interpolationActivated(InterpolationType interp);

private slots:
   void translateActivated(int index);

private:
   InterpolationComboBox(const InterpolationComboBox& rhs);
   InterpolationComboBox& operator=(const InterpolationComboBox& rhs);
};

#endif
