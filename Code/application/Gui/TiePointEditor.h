/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTEDITOR_H
#define TIEPOINTEDITOR_H

#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QtGui/QTableView>

#include "SafePtr.h"
#include "TiePointLayer.h"
#include "TiePointList.h"

#include <vector>

class Layer;
class TiePointTableModel;

class TiePointEditor : public QDialog
{
   Q_OBJECT

public:
   TiePointEditor(QWidget* pParent = 0);
   ~TiePointEditor();

public slots:
   bool setTiePointLayer(Layer* pLayer);

signals:
   void visibilityChanged(bool);

protected slots:
   void addPoint();
   void deletePoint();
   void goToRow();
   void goToRow(int row);
   void pointsEdited(const std::vector<TiePoint>& oldPoints, const std::vector<TiePoint>& newPoints);

protected:
   void showEvent(QShowEvent* pEvent);
   void hideEvent(QHideEvent* pEvent);

private:
   QTableView* mpTableView;
   QPushButton* mpAddButton;
   QPushButton* mpDeleteButton;
   QPushButton* mpGoToButton;

   SafePtr<TiePointLayer> mpLayer;
   TiePointTableModel* mpTableModel;

   void enableButtons(bool bEnable);
};

#endif
