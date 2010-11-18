/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef SIGNATUREPROPERTIESDLG_H
#define SIGNATUREPROPERTIESDLG_H

#include <QtGui/QDialog>

class Signature;

/**
 *  A dialog to display signature properties.
 *
 *  The signature properties dialog displays the following information about a
 *  Signature object:
 *  - Name
 *  - Data set name
 *  - %Classification
 *  - Acquisition method
 *  - Acquisition date
 *  - Validation source
 *  - %Description
 *  - Metadata
 *
 *  @see        Signature
 */
class SignaturePropertiesDlg : public QDialog
{
   Q_OBJECT

public:
   /**
    *  Creates and populates the dialog for a given signature.
    *
    *  The constructor creates the widgets and initializes the values for the
    *  given signature.
    *
    *  @param   pSignature
    *           The signature for which to display its properties.
    *  @param   parent
    *           The parent widget.
    */
   SignaturePropertiesDlg(Signature* pSignature, QWidget* parent = 0);

   /**
    *  Destroys the dialog.
    */
   ~SignaturePropertiesDlg();
};

#endif
