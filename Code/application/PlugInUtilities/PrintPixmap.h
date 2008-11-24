/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRINTPIXMAP_H
#define PRINTPIXMAP_H

#include <QtGui/QPixmap>

class QWidget;
class QPrinter;

/* Prints a QPixmap, optionally displaying the print setup dialog first */
void PrintPixmap(QPixmap pixmap, bool displayDialog = false, QWidget* pParent = NULL, QPrinter* pPrinter = NULL);

#endif
