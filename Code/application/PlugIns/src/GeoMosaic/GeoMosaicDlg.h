/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOMOSAICDLG_H
#define GEOMOSAICDLG_H

#include "MosaicManager.h"
#include "ProgressTracker.h"

#include <QtGui/QDialog>

class Progress;
class QCheckBox;
class QDialogButtonBox;
class QListWidget;

namespace boost
{
   class any;
}

class GeoMosaicDlg : public QDialog
{
   Q_OBJECT

public:
   GeoMosaicDlg(Progress* pProgress, QWidget* pParent = NULL);
   ~GeoMosaicDlg();

protected slots:
   void accept();
   void reject();
   void loadData();
   void enableOK();

private:
   void batchStitch();

   QDialogButtonBox* mpDlgBtns;
   QListWidget* mpPrimaryList;
   QCheckBox* mpCreateAnimationCheckBox;
   ProgressTracker mProgressTracker;
};

#endif
