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
#include <QtGui/QScrollBar>
#include <QtGui/QTableWidget>

#include <boost/any.hpp>
#include <vector>

#include "TypesFile.h"

class Layer;
class Subject;
class TiePointLayer;
class TiePointList;

class TiePointEditor : public QDialog
{
   Q_OBJECT

public:
   TiePointEditor(QWidget* pParent = 0);
   ~TiePointEditor();

   void elementDeleted(Subject &subject, const std::string &signal, const boost::any &data);
   void elementModified(Subject &subject, const std::string &signal, const boost::any &data);

public slots:
   bool setTiePointLayer(Layer* pLayer);

signals:
   void visibilityChanged(bool);

protected slots:
   void goToRow();
   void goToRow(int row);
   void addPoint();
   void updatePoint(int row, int column);
   void deletePoint();
   void updateTiePointTable();

protected:
   void resizeEvent(QResizeEvent *pEvent);
   bool eventFilter(QObject* o, QEvent* e);
   unsigned int numVisibleRows() const;
   void showEvent(QShowEvent* e);
   void closeEvent(QCloseEvent* e);

   void tiePointListDeleted(Subject& subject, const std::string& signal, const boost::any& value);

private:
   QTableWidget* mpTable;
   QPushButton* mpAddButton;
   QPushButton* mpDeleteButton;
   QPushButton* mpGoToButton;
   QPushButton* mpCloseButton;
   QScrollBar* mpScrollBar;

   TiePointLayer* mpLayer;
   TiePointList *mpTiePointList;

   void enableButtons(bool bEnable);
};

#endif
