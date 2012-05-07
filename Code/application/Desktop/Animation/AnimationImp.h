/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANIMATIONIMP_H
#define ANIMATIONIMP_H

#include <QtCore/QObject>

#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "AnimationFrame.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class AnimationImp : public QObject, public SessionItemImp, public SubjectImp
{
   Q_OBJECT

public:
   typedef std::vector<AnimationFrame>::iterator FrameIterator;

   AnimationImp(FrameType frameType, const std::string& id);
   virtual ~AnimationImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;
   bool attach(const std::string& signal, const Slot& slot);
   bool detach(const std::string& signal, const Slot& slot);

   void setName(const std::string& name);

   FrameType getFrameType() const;

   void setFrames(const std::vector<AnimationFrame>& frames);
   const std::vector<AnimationFrame>& getFrames() const;
   unsigned int getNumFrames() const;
   bool hasFrame(const AnimationFrame* pFrame) const;
   void setCurrentFrame(const AnimationFrame* pFrame);
   void setCurrentFrame(double frameValue);
   void setCurrentFrame(FrameIterator frameIter);
   const AnimationFrame* getCurrentFrame() const;

   double getStartValue() const;
   double getStopValue() const;
   double getNextFrameValue(AnimationState direction, size_t offset = 1) const;

   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

signals:
   void renamed(const QString& strName);
   void framesChanged(const std::vector<AnimationFrame>& frames);
   void objectAttached();
   void objectDetached();

protected:
   double getFrameValue(const AnimationFrame* pFrame) const;

private:
   AnimationImp(const AnimationImp& rhs);
   AnimationImp& operator=(const AnimationImp& rhs);

   FrameType mFrameType;
   std::vector<AnimationFrame> mFrames;
   FrameIterator mCurrentFrameIter;
};

#define ANIMATIONADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define ANIMATIONADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   void setName(const std::string& name) \
   { \
      return impClass::setName(name); \
   } \
   FrameType getFrameType() const \
   { \
      return impClass::getFrameType(); \
   } \
   void setFrames(const std::vector<AnimationFrame>& frames) \
   { \
      return impClass::setFrames(frames); \
   } \
   const std::vector<AnimationFrame>& getFrames() const \
   { \
      return impClass::getFrames(); \
   } \
   unsigned int getNumFrames() const \
   { \
      return impClass::getNumFrames(); \
   } \
   bool hasFrame(const AnimationFrame* pFrame) const \
   { \
      return impClass::hasFrame(pFrame); \
   } \
   using impClass::setCurrentFrame; \
   void setCurrentFrame(const AnimationFrame* pFrame) \
   { \
      return impClass::setCurrentFrame(pFrame); \
   } \
   void setCurrentFrame(double frameValue) \
   { \
      return impClass::setCurrentFrame(frameValue); \
   } \
   const AnimationFrame* getCurrentFrame() const \
   { \
      return impClass::getCurrentFrame(); \
   } \
   double getStartValue() const \
   { \
      return impClass::getStartValue(); \
   } \
   double getStopValue() const \
   { \
      return impClass::getStopValue(); \
   } \
   double getNextFrameValue(AnimationState direction, size_t offset = 1) const \
   { \
      return impClass::getNextFrameValue(direction, offset); \
   }

#endif
