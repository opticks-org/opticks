/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICUNITSWIDGET_H
#define GRAPHICUNITSWIDGET_H

#include <QtGui/QWidget>
#include "TypesFile.h"

class QComboBox;

class GraphicUnitsWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicUnitsWidget(QWidget* pParent = NULL);
   UnitSystem getUnitSystem() const;

public slots:
   void setUnitSystem(UnitSystem units);

signals:
   void unitSystemChanged(UnitSystem value);

private slots:
   void translateActivated(int newIndex);

private:
   GraphicUnitsWidget(const GraphicUnitsWidget& rhs);
   GraphicUnitsWidget& operator=(const GraphicUnitsWidget& rhs);
   QComboBox* mpUnitSystemComboBox;
};

#endif
