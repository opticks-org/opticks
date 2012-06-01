/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IGMGUI_H
#define IGMGUI_H

#include "AttachmentPtr.h"
#include "GeoreferenceDescriptor.h"

#include <QtGui/QWidget>

#include <string>

class FileBrowser;
class QRadioButton;

class IgmGui : public QWidget
{
   Q_OBJECT

public:
   IgmGui(QWidget* pParent = NULL);
   virtual ~IgmGui();

   void setGeoreferenceData(GeoreferenceDescriptor* pDescriptor, bool enableExistingElement, bool enableIgmFilename);
   GeoreferenceDescriptor* getGeoreferenceDescriptor();
   const GeoreferenceDescriptor* getGeoreferenceDescriptor() const;

protected:
   void georeferenceDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);
   void updateFromGeoreferenceDescriptor();

protected slots:
   void setExistingElement(bool useExistingElement);
   void setIgmFilename(const QString& filename);

private:
   IgmGui(const IgmGui& rhs);
   IgmGui& operator=(const IgmGui& rhs);

   AttachmentPtr<GeoreferenceDescriptor> mpDescriptor;

   QRadioButton* mpUseExisting;
   QRadioButton* mpLoad;
   FileBrowser* mpFilename;
};

#endif
