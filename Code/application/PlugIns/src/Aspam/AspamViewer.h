/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAMVIEWER_H
#define ASPAMVIEWER_H

#include "AspamManager.h"
#include "AttachmentPtr.h"
#include "ModelServices.h"
#include "ViewerShell.h"

#include <boost/any.hpp>
#include <string>

class AspamViewerDialog;
class ApplicationServices;

/**
 *  Plug-in for viewing and plotting ASPAM data.
 */
class AspamViewer : public ViewerShell
{
public:
   AspamViewer();
   ~AspamViewer();

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   bool execute(PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList);
   QWidget* getWidget() const;
   void sessionClosing(Subject &pSubject, const std::string &signal, const boost::any &data);

   /**
    *  This slot updates the viewer dialog when aspams are added or modified.
    *
    *  @param subject
    *         The object which sent the signal which is calling this slot.
    *
    *  @param signal
    *         The name of the signal which is calling this slot.
    *
    *  @param data
    *         Data associated with the signal.
    */
   void updateAspams(Subject &subject, const std::string &signal, const boost::any &data);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

private:
   AspamViewerDialog *mpMainWindow;
   Service<ModelServices> mpModelServices;
   AttachmentPtr<AspamManager> mpMgrAttachment;
   AttachmentPtr<ModelServices> mpModelAttachment;
   AttachmentPtr<ApplicationServices> mpAppSrvcsAttachment;
   bool mbSessionClosing;
};

#endif // ASPAMVIEWER_H