/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SAMPLEGEOREFGUI_H
#define SAMPLEGEOREFGUI_H

#include "AttachmentPtr.h"
#include "GeoreferenceDescriptor.h"

#include <QtGui/QWidget>

#include <string>

class QCheckBox;
class QSpinBox;

class SampleGeorefGui : public QWidget
{
   Q_OBJECT

public:
   SampleGeorefGui(QWidget* pParent = NULL);
   virtual ~SampleGeorefGui();

   void setGeoreferenceDescriptor(GeoreferenceDescriptor* pDescriptor);
   GeoreferenceDescriptor* getGeoreferenceDescriptor();
   const GeoreferenceDescriptor* getGeoreferenceDescriptor() const;

protected:
   void georeferenceDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);
   void updateFromGeoreferenceDescriptor();

protected slots:
   void setXSize(int xSize);
   void setYSize(int ySize);
   void setExtrapolate(bool extrapolate);

private:
   SampleGeorefGui(const SampleGeorefGui& rhs);
   SampleGeorefGui& operator=(const SampleGeorefGui& rhs);

   AttachmentPtr<GeoreferenceDescriptor> mpDescriptor;

   QSpinBox* mpXSpin;
   QSpinBox* mpYSpin;
   QCheckBox* mpExtrapolateCheck;
};

#endif
