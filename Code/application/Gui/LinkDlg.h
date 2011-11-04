/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LINKDLG_H
#define LINKDLG_H

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>

class Layer;
class LinkDatasetPage;
class LinkOptionsPage;
class SpatialDataView;

class LinkDlg : public QDialog
{
   Q_OBJECT

public:
   LinkDlg(QWidget* parent = 0);
   ~LinkDlg();

   static Layer* duplicateLayer(Layer* pLayer, SpatialDataView* pView);

protected:
   void showPage(QWidget* pPage);

protected slots:
   void back();
   void next();
   void accept();

private:
   LinkDlg(const LinkDlg& rhs);
   LinkDlg& operator=(const LinkDlg& rhs);
   QLabel* mpDescriptionLabel;
   QStackedWidget* mpStack;
   LinkDatasetPage* mpSourcePage;
   LinkOptionsPage* mpOptionsPage;
   LinkDatasetPage* mpDestinationPage;
   QPushButton* mpBackButton;
   QPushButton* mpNextButton;
   QPushButton* mpFinishButton;
};

#endif
