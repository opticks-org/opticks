/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FUSIONALGORITHMINPUTSPAGE_H
#define FUSIONALGORITHMINPUTSPAGE_H

#include <QtGui/QCheckBox>

#include "AoiLayer.h"
#include "FusionPage.h"
#include "Observer.h"

#include <boost/any.hpp>

class QCheckBox;
class QLabel;
class QGroupBox;
class QPushButton;
class SpatialDataView;
class Subject;

class FusionAlgorithmInputsPage : public FusionPage, public Observer
{
   Q_OBJECT
public:
   FusionAlgorithmInputsPage(QWidget* pParent);
   ~FusionAlgorithmInputsPage();

   void aoiModified(Subject& subject, const std::string& signal, const boost::any& v);
   void aoiLayerDeleted(Subject& subject, const std::string& signal, const boost::any& v);
   void aoiLayerAttached(Subject& subject, const std::string& signal, const boost::any& v);
   void aoiLayerDetached(Subject& subject, const std::string& signal, const boost::any& v);
   void attached(Subject& subject, const std::string& signal, const Slot& slot);
   void detached(Subject& subject, const std::string& signal, const Slot& slot);

   bool isValid() const;

   void setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary);

   bool sbs() const;
   bool flicker() const;
   bool openOverlayTools() const;

   bool getRoiBoundingBox(int& x1, int& y1, int& x2, int& y2,
      int rasterBbX1, int rasterBbY1, int rasterBbX2, int rasterBbY2) const;

   bool inMemory() const;

   std::string getPreferredPrimaryMouseMode() const;
   Layer* getPreferredPrimaryActiveLayer() const;

   bool copyColormap(const SpatialDataView& view);

protected:
   void createFusionAoiLayer(SpatialDataView* pView);
   void setAoiLayer(AoiLayer* pLayer);

   void showEvent(QShowEvent* pEvt);

signals:
   void executeAlgorithm();

protected slots:
   void enableFusion();

private:
   static const std::string FUSION_ROI_NAME;

   QLabel* mpCurrentMinX;
   QLabel* mpCurrentMinY;
   QLabel* mpCurrentMaxX;
   QLabel* mpCurrentMaxY;
   QGroupBox* mpProductGroup;
   QCheckBox* mpSbsOption;
   QCheckBox* mpFlickerOption;
   QCheckBox* mpRunOverlayOption;
   QCheckBox* mpPrimaryCheck;
   QCheckBox* mpSecondaryCheck;
   QPushButton* mpExecuteButton;

   AoiLayer* mpAoiLayer;

   QCheckBox* mpOnDiskButton;
};

#endif
