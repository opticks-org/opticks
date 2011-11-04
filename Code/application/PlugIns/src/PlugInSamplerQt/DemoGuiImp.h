/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DEMOGUIIMP_H
#define DEMOGUIIMP_H

#include "ui_DynamicColormap.h"
#include "TypesFile.h"
#include "Node.h"

#include <vector>

class AlgorithmRunner;

/**
 *  Back-end implementation of the Demo PlugIn Gui, providing the interface
 *  between the user and the Dynamic Colormap algorithm.
 *
 *  The DemoGuiImp class derives from the DynamicColormap class. The
 *  DynamicColormap is built from a ui file and creates all GUI elements.
 *  The DemoGuiImp implements all callbacks and initialization.
 */
class DemoGuiImp : public QDialog, public Ui::DynamicColormap
{
   Q_OBJECT
public:
   DemoGuiImp( QWidget* pParent, AlgorithmRunner *pRunner);

   std::vector<PlugInSamplerQt::Node> &getNodes();
   bool getModified();

public slots:
   void accept();
   void apply();

private:
   DemoGuiImp(const DemoGuiImp& rhs);
   DemoGuiImp& operator=(const DemoGuiImp& rhs);

   void setButtonColors();

   AlgorithmRunner* mpRunner;
   QColor mColor1;
   QColor mColor2;
   QColor mColor3;
   QColor mColor4;
};

#endif
