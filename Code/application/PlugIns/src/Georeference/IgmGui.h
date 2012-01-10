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

#include "DesktopServices.h"
#include "LabeledSectionGroup.h"
#include "ModelServices.h"

#include <string>
#include <vector>

class FileBrowser;
class Progress;
class RasterElement;
class QRadioButton;

class IgmGui : public QWidget
{
   Q_OBJECT

public:
   IgmGui(RasterElement* pRasterElement, QWidget* pParent = 0);
   ~IgmGui();
   bool validateInput();
   bool useExisting() const;
   void hasExisting(bool val);
   QString getFilename() const;

protected:
   QRadioButton* mpUseExisting;
   QRadioButton* mpLoad;
   FileBrowser* mpFilename;

private:
   IgmGui(const IgmGui& rhs);
   IgmGui& operator=(const IgmGui& rhs);
};

#endif // IGMGUI_H
