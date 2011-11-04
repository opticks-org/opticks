/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GCPGUI_H
#define GCPGUI_H

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

#include "ModelServices.h"

#include <string>
#include <vector>

class RasterElement;

class GcpGui : public QWidget
{
   Q_OBJECT

public:
   GcpGui(int maxOrder, const std::vector<std::string>& gcpLists, RasterElement* pRasterElement,
      QWidget* pParent = 0);
   ~GcpGui();

   unsigned short getOrder() const;
   std::string getGcpListName() const;
   bool validateInput();

protected slots:
   bool validateOrder(int iOrder);
   bool validateGcpList(const QString& strGcpList);
   bool setCurrentGcpList(const QString& strGcpList);

private:
   GcpGui(const GcpGui& rhs);
   GcpGui& operator=(const GcpGui& rhs);

   const int mMaxOrder;
   Service<ModelServices> mpModel;
   RasterElement* mpRasterElement;

   int getMaxOrder(int numGcps);

   QComboBox* mpGcpListCombo;
   QSpinBox* mpOrderSpin;
   QLabel* mpOrderLabel;
};

#endif // GCPGUI_H
