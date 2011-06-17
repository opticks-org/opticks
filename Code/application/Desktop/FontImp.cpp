/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FontImp.h"
#include "StringUtilities.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

FontImp::FontImp()
{
}

FontImp::FontImp(const QFont& font) :
   mFont(font)
{
}

FontImp::~FontImp()
{
}

FontImp& FontImp::operator=(const FontImp& font)
{
   if (this != &font)
   {
      mFont = font.mFont;
   }

   return *this;
}

bool FontImp::operator==(const FontImp& font) const
{
   return (mFont == font.mFont);
}

bool FontImp::operator!=(const FontImp& font) const
{
   return (mFont != font.mFont);
}

void FontImp::setFace(const string& face)
{
   if (face.empty() == false)
   {
      mFont.setFamily(QString::fromStdString(face));
   }
}

string FontImp::getFace() const
{
   return mFont.family().toStdString();
}

void FontImp::setPointSize(int iSize)
{
   mFont.setPointSize(iSize);
}

int FontImp::getPointSize() const
{
   return mFont.pointSize();
}

void FontImp::setBold(bool bBold)
{
   mFont.setBold(bBold);
}

bool FontImp::getBold() const
{
   return mFont.bold();
}

void FontImp::setItalic(bool bItalic)
{
   mFont.setItalic(bItalic);
}

bool FontImp::getItalic() const
{
   return mFont.italic();
}

void FontImp::setUnderline(bool bUnderline)
{
   mFont.setUnderline(bUnderline);
}

bool FontImp::getUnderline() const
{
   return mFont.underline();
}

void FontImp::setQFont(const QFont& font)
{
   mFont = font;
}

QFont FontImp::toQFont() const
{
   return mFont;
}

QFont& FontImp::getQFont()
{
   return mFont;
}

const QFont& FontImp::getQFont() const
{
   return mFont;
}

bool FontImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("face", getFace());
   pXml->addAttr("pointSize", getPointSize());
   pXml->addAttr("bold", getBold());
   pXml->addAttr("italic", getItalic());
   pXml->addAttr("underline", getUnderline());
   return true;
}

bool FontImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement* pElmnt = static_cast<DOMElement*>(pDocument);
   if (pElmnt == NULL)
   {
      return false;
   }

   setFace(A(pElmnt->getAttribute(X("face"))));
   setPointSize(StringUtilities::fromXmlString<int>(A(pElmnt->getAttribute(X("pointSize")))));
   setBold(StringUtilities::fromXmlString<bool>(A(pElmnt->getAttribute(X("bold")))));
   setItalic(StringUtilities::fromXmlString<bool>(A(pElmnt->getAttribute(X("italic")))));
   setUnderline(StringUtilities::fromXmlString<bool>(A(pElmnt->getAttribute(X("underline")))));

   return true;
}
