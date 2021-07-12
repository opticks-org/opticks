/*
 * The information in this file is
 * Copyright(c) 2021 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERELEMENTUPDATER_H__
#define RASTERELEMENTUPDATER_H__

#include "RasterElement.h"
#include <QtCore/QObject>

/**
 *  This just provides a Qt slot to call RasterElement::updateData().
 *  It's sole reason for existing is to keep Qt out of the core data model.
 */
class RasterElementUpdater : public QObject
{
   Q_OBJECT

public:
   RasterElementUpdater(RasterElement* pElement, QObject* pParent=nullptr);
   virtual ~RasterElementUpdater();

public slots:
   void updateData();

private:
   RasterElement* mpElement;
};

#endif
