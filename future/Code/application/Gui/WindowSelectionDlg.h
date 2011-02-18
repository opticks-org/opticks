/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef WINDOWSELECTIONDLG_H
#define WINDOWSELECTIONDLG_H

#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QRadioButton>

#include "TypesFile.h"

#include <vector>

class WorkspaceWindow;

class WindowSelectionDlg : public QDialog
{
   Q_OBJECT

public:
   WindowSelectionDlg(QWidget* parent = 0);
   ~WindowSelectionDlg();

   std::vector<WorkspaceWindow*> getSelectedWindows() const;
   TilingType getTilingType() const;
   bool maximizeFirstColumn();

   QSize sizeHint() const;

protected slots:
   void accept();
   void clearSelections();
   void addSelection();
   void removeSelection();
   void tileChanged(bool enable);

private:
   QListWidget* mpTileList;
   QListWidget* mpSelectedList;
   QRadioButton* mpTileRadio;
   QRadioButton* mpTileHRadio;
   QRadioButton* mpTileVRadio;
   QCheckBox* mpMaxFirst;
};

#endif
