/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BANDBINNINGDLG_H
#define BANDBINNINGDLG_H

#include <QtGui/QDialog>

#include "DimensionDescriptor.h"

#include <utility>
#include <vector>

class BandBinningModel;
class QKeyEvent;
class QTableView;
class RasterDataDescriptor;

class BandBinningDlg : public QDialog
{
   Q_OBJECT

public:
   BandBinningDlg(const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands,
      const RasterDataDescriptor* pDescriptor, QWidget* pParent = NULL);
   virtual ~BandBinningDlg();

   const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& getGroupedBands();

public slots:
   virtual void accept();

protected slots:
   virtual void openGroupedBands();
   virtual void saveGroupedBands();
   virtual void addGroupedBand();
   virtual void deleteGroupedBands();
   virtual void clearGroupedBands();
   virtual void resetGroupedBands();
   virtual void moveUpGroupedBands();
   virtual void moveDownGroupedBands();

protected:
   virtual void keyPressEvent(QKeyEvent* pEvent);

private:
   QTableView* mpGroupedBandView;
   BandBinningModel* mpBandModel;
   const RasterDataDescriptor* mpDescriptor;
   const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& mOriginalGroupedBands;
};

#endif
