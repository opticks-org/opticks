/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAMWINDOWADAPTER_H
#define HISTOGRAMWINDOWADAPTER_H

#include "HistogramWindow.h"
#include "HistogramWindowImp.h"

class HistogramWindowAdapter : public HistogramWindow, public HistogramWindowImp
{
public:
   HistogramWindowAdapter(const std::string& id, QWidget* parent = 0);
   ~HistogramWindowAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   HISTOGRAMWINDOWADAPTER_METHODS(HistogramWindowImp)
};

#endif
