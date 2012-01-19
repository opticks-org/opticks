/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ASPAMIMP_H
#define ASPAMIMP_H

#include "Aspam.h"
#include "SubjectImp.h"

class StepResource;

/**
 *  Default implementation for the Aspam data model.
 *
 *  This implementation follows and adapter pattern. The adapter
 *  implements the Aspam interface and the AspamImp implementation
 *  class. This allows complete separation of the interface from
 *  the implementation and ensure binary compatibility if the
 *  implementation changes.
 */
class AspamImp : public SubjectImp
{
public:
   AspamImp();
   ~AspamImp();

   virtual const Aspam::ParagraphA& getParagraphA() const;
   virtual const Aspam::ParagraphB& getParagraphB() const;
   virtual const Aspam::ParagraphC& getParagraphC() const;
   virtual const Aspam::ParagraphD& getParagraphD() const;
   virtual const Aspam::ParagraphE& getParagraphE() const;
   virtual const Aspam::ParagraphF& getParagraphF() const;
   virtual const Aspam::ParagraphG& getParagraphG() const;
   virtual const Aspam::ParagraphH& getParagraphH() const;
   virtual const Aspam::ParagraphI& getParagraphI() const;
   virtual const Aspam::ParagraphJ& getParagraphJ() const;
   virtual const Aspam::ParagraphK& getParagraphK() const;

   virtual void setParagraphA(const Aspam::ParagraphA& val);
   virtual void setParagraphB(const Aspam::ParagraphB& val);
   virtual void setParagraphC(const Aspam::ParagraphC& val);
   virtual void setParagraphD(const Aspam::ParagraphD& val);
   virtual void setParagraphE(const Aspam::ParagraphE& val);
   virtual void setParagraphF(const Aspam::ParagraphF& val);
   virtual void setParagraphG(const Aspam::ParagraphG& val);
   virtual void setParagraphH(const Aspam::ParagraphH& val);
   virtual void setParagraphI(const Aspam::ParagraphI& val);
   virtual void setParagraphJ(const Aspam::ParagraphJ& val);
   virtual void setParagraphK(const Aspam::ParagraphK& val);

   AnyData* copy() const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

private:
   Aspam::ParagraphA mParagraphA;
   Aspam::ParagraphB mParagraphB;
   Aspam::ParagraphC mParagraphC;
   Aspam::ParagraphD mParagraphD;
   Aspam::ParagraphE mParagraphE;
   Aspam::ParagraphF mParagraphF;
   Aspam::ParagraphG mParagraphG;
   Aspam::ParagraphH mParagraphH;
   Aspam::ParagraphI mParagraphI;
   Aspam::ParagraphJ mParagraphJ;
   Aspam::ParagraphK mParagraphK;
};

#define ASPAMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

/**
 *  This makes it easy to write the adapter class.
 */
#define ASPAMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   const Aspam::ParagraphA& getParagraphA() const \
   { \
      return impClass::getParagraphA(); \
   } \
   const Aspam::ParagraphB& getParagraphB() const \
   { \
      return impClass::getParagraphB(); \
   } \
   const Aspam::ParagraphC& getParagraphC() const \
   { \
      return impClass::getParagraphC(); \
   } \
   const Aspam::ParagraphD& getParagraphD() const \
   { \
      return impClass::getParagraphD(); \
   } \
   const Aspam::ParagraphE& getParagraphE() const \
   { \
      return impClass::getParagraphE(); \
   } \
   const Aspam::ParagraphF& getParagraphF() const \
   { \
      return impClass::getParagraphF(); \
   } \
   const Aspam::ParagraphG& getParagraphG() const \
   { \
      return impClass::getParagraphG(); \
   } \
   const Aspam::ParagraphH& getParagraphH() const \
   { \
      return impClass::getParagraphH(); \
   } \
   const Aspam::ParagraphI& getParagraphI() const \
   { \
      return impClass::getParagraphI(); \
   } \
   const Aspam::ParagraphJ& getParagraphJ() const \
   { \
      return impClass::getParagraphJ(); \
   } \
   const Aspam::ParagraphK& getParagraphK() const \
   { \
      return impClass::getParagraphK(); \
   } \
   void setParagraphA(const Aspam::ParagraphA& val) \
   { \
      return impClass::setParagraphA(val); \
   } \
   void setParagraphB(const Aspam::ParagraphB& val) \
   { \
      return impClass::setParagraphB(val); \
   } \
   void setParagraphC(const Aspam::ParagraphC& val) \
   { \
      return impClass::setParagraphC(val); \
   } \
   void setParagraphD(const Aspam::ParagraphD& val) \
   { \
      return impClass::setParagraphD(val); \
   } \
   void setParagraphE(const Aspam::ParagraphE& val) \
   { \
      return impClass::setParagraphE(val); \
   } \
   void setParagraphF(const Aspam::ParagraphF& val) \
   { \
      return impClass::setParagraphF(val); \
   } \
   void setParagraphG(const Aspam::ParagraphG& val) \
   { \
      return impClass::setParagraphG(val); \
   } \
   void setParagraphH(const Aspam::ParagraphH& val) \
   { \
      return impClass::setParagraphH(val); \
   } \
   void setParagraphI(const Aspam::ParagraphI& val) \
   { \
      return impClass::setParagraphI(val); \
   } \
   void setParagraphJ(const Aspam::ParagraphJ& val) \
   { \
      return impClass::setParagraphJ(val); \
   } \
   void setParagraphK(const Aspam::ParagraphK& val) \
   { \
      return impClass::setParagraphK(val); \
   }

#endif
