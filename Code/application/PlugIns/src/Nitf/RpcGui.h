/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RPCGUI_H
#define RPCGUI_H

#include "AttachmentPtr.h"
#include "GeoreferenceDescriptor.h"

#include <QtGui/QWidget>

class QSpinBox;

class RpcGui : public QWidget
{
   Q_OBJECT

public:
   RpcGui(QWidget* pParent = NULL);
   virtual ~RpcGui();

   void setGeoreferenceDescriptor(GeoreferenceDescriptor* pDescriptor);
   GeoreferenceDescriptor* getGeoreferenceDescriptor();
   const GeoreferenceDescriptor* getGeoreferenceDescriptor() const;

protected:
   void georeferenceDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);
   void updateFromGeoreferenceDescriptor();

protected slots:
   void setHeight(int heightValue);

private:
   RpcGui(const RpcGui& rhs);
   RpcGui& operator=(const RpcGui& rhs);

   AttachmentPtr<GeoreferenceDescriptor> mpDescriptor;
   QSpinBox* mpHeightSpin;
};

#endif
