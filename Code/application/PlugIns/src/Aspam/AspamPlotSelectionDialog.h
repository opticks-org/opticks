/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAMPLOTSELECTIONDIALOG_H
#define ASPAMPLOTSELECTIONDIALOG_H

#include <QtCore/QString>
#include <QtGui/QDialog>

class QComboBox;
class QStringList;

/**
 *  Qt dialog box which allows the user to choose plotting options when plotting ASPAM data.
 *
 *  @todo Use the new DataVariant plotting capabilities instead of custom plotting.
 */
class AspamPlotSelectionDialog : public QDialog
{
   Q_OBJECT

public:
   /**
    *  Create a new plot selection dialog.
    *
    *  @param choices
    *         List of variable names which can be the primary axis variable.
    *
    *  @param pParent
    *         The parent widget.
    */
   AspamPlotSelectionDialog(QStringList& choices, QWidget* pParent = NULL);

   ~AspamPlotSelectionDialog();

   /**
    *  Accessor for the variable name which is to be used as the primary axis.
    *
    *  @return The primary axis variable name.
    */
   QString getPrimaryAxis() const;

   /**
    *  Accessor for the name of the plot that the data should be placed on.
    *
    *  @return The name of an existing plot, or New Plot.
    */
   QString getPlotName() const;

   /**
    *  Set the plot name choices.
    *
    *  @param names
    *         Plot names available. "New Plot" will be automatically prepended to this list.
    *
    *  @param selected
    *         The plot name which should be selected by default. If this is empty or not
    *         in the names list, "New Plot" will be selected.
    */
   void setPlotNames(const QStringList& names, const QString& selected);

private:
   AspamPlotSelectionDialog(const AspamPlotSelectionDialog& rhs);
   AspamPlotSelectionDialog& operator=(const AspamPlotSelectionDialog& rhs);
   QComboBox* mpPrimary;
   QComboBox* mpPlotName;
};

#endif
