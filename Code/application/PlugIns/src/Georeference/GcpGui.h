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

#include "AttachmentPtr.h"
#include "GcpList.h"
#include "GeoreferenceDescriptor.h"

#include <QtGui/QWidget>

#include <list>
#include <string>

class QComboBox;
class QLabel;
class QSpinBox;
class RasterElement;

class GcpGui : public QWidget
{
   Q_OBJECT

public:
   GcpGui(int maxOrder, QWidget* pParent = NULL);
   virtual ~GcpGui();

   void setGeoreferenceData(GeoreferenceDescriptor* pDescriptor, const std::list<GcpPoint>& gcps);
   void setGeoreferenceData(GeoreferenceDescriptor* pDescriptor, const RasterElement* pRaster);
   GeoreferenceDescriptor* getGeoreferenceDescriptor();
   const GeoreferenceDescriptor* getGeoreferenceDescriptor() const;

protected:
   void georeferenceDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);
   void updateFromGeoreferenceDescriptor();

   int getMaxOrder(int numGcps) const;
   void updateOrderRange();
   void updateOrderLabel();

protected slots:
   void setGcpList(const QString& gcpList);
   void setOrder(int order);

private:
   GcpGui(const GcpGui& rhs);
   GcpGui& operator=(const GcpGui& rhs);

   const int mMaxOrder;
   AttachmentPtr<GeoreferenceDescriptor> mpDescriptor;
   std::list<GcpPoint> mGcps;
   const RasterElement* mpRasterElement;

   QLabel* mpGcpListLabel;
   QComboBox* mpGcpListCombo;
   QSpinBox* mpOrderSpin;
   QLabel* mpOrderLabel;
};

#endif
