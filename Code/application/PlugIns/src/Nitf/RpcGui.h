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

#include <QtGui/QWidget>

class QSpinBox;
class RasterElement;

class RpcGui : public QWidget
{
   Q_OBJECT

public:
   RpcGui(RasterElement* pRasterElement, QWidget* pParent = 0);
   ~RpcGui();
   int getHeightSize() const;
   bool validateInput();

private:
   RasterElement* mpRasterElement;
   QSpinBox* mpHeightSpin;
};

#endif // RPCGUI_H
