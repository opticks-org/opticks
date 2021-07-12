/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "RasterElement.h"
#include "RasterElementUpdater.h"

RasterElementUpdater::RasterElementUpdater(RasterElement* pElement, QObject* pParent) : QObject(pParent), mpElement(pElement)
{
   ENSURE(pElement != nullptr);
}

RasterElementUpdater::~RasterElementUpdater()
{}

void RasterElementUpdater::updateData()
{
   mpElement->updateData();
   deleteLater();
}
