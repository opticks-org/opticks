/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FONTIMP_H
#define FONTIMP_H

#include <QtGui/QFont>

#include "Font.h"
#include "Serializable.h"

class FontImp : public Font, public Serializable
{
public:
   FontImp();
   FontImp(const QFont& font);
   ~FontImp();

   FontImp& operator= (const FontImp& font);
   bool operator== (const FontImp& font) const;
   bool operator!= (const FontImp& font) const;

   void setFace(const std::string& face);
   std::string getFace() const;
   void setPointSize(int iSize);
   int getPointSize() const;

   void setBold(bool bBold);
   bool getBold() const;
   void setItalic(bool bItalic);
   bool getItalic() const;
   void setUnderline(bool bUnderline);
   bool getUnderline() const;

   void setQFont(const QFont& font);
   QFont toQFont() const;
   QFont& getQFont();
   const QFont& getQFont() const;

   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   QFont mFont;
};

#endif
