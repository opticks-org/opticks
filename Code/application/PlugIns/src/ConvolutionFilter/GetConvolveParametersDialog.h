/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GETCONVOLVEPARAMETERSDIALOG_H__
#define GETCONVOLVEPARAMETERSDIALOG_H__

#include <QtGui/QDialog>
#include <vector>

class AoiElement;
class RasterElement;
class SpatialDataView;

class QComboBox;
class QListWidget;

class GetConvolveParametersDialog : public QDialog
{
   Q_OBJECT

public:
   GetConvolveParametersDialog(SpatialDataView* pView,
      RasterElement* pElement, QWidget* pParent = NULL);
   virtual ~GetConvolveParametersDialog();

   void setSelectedAoi(const AoiElement* pElement);
   AoiElement* getSelectedAoi() const;
   void setBandSelectionIndices(const std::vector<unsigned int>& indices);
   std::vector<unsigned int> getBandSelectionIndices() const;

private:
   GetConvolveParametersDialog(const GetConvolveParametersDialog& rhs);
   GetConvolveParametersDialog& operator=(const GetConvolveParametersDialog& rhs);
   QComboBox* mpAoiSelect;
   QListWidget* mpBandSelect;
   SpatialDataView* mpView;
};

#endif