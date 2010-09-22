/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHUNITSCOMBOBOX_H
#define WAVELENGTHUNITSCOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

/**
 *  A combo box that will allow a user to select wavelength units.
 *
 *  This is a read-only combo box that provides selection values based on the
 *  WavelengthUnitsType enum values.
 */
class WavelengthUnitsComboBox : public QComboBox
{
   Q_OBJECT

public:
   /**
    *  Creates a new wavelength units combo box.
    *
    *  @param   pParent
    *           The parent widget.
    */
   WavelengthUnitsComboBox(QWidget* pParent = NULL);

   /**
    *  Destroys the combo box.
    */
   ~WavelengthUnitsComboBox();

   /**
    *  Sets the selected wavelength units.
    *
    *  @param   units
    *           The wavelength units to select.
    */
   void setUnits(WavelengthUnitsType units);

   /**
    *  Returns the selected wavelength units.
    *
    *  @return  The selected wavelength units.
    */
   WavelengthUnitsType getUnits() const;

signals:
   /**
    *  Emitted when the user selects a units value in the combo box.
    *
    *  This signal is not emitted if the selected units change when setUnits()
    *  is called.
    *
    *  @param   units
    *           The newly selected units in the combo box.
    */
   void unitsActivated(WavelengthUnitsType units);

private slots:
   void translateActivated(int index);
};

#endif
