/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDateTime>

#include "Animation.h"
#include "AnimationImp.h"
#include "AppVerify.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <algorithm>
#include <math.h>
using namespace std;
XERCES_CPP_NAMESPACE_USE

namespace
{
   class AnimationFrameLessThanEqual
   {
   public:
      AnimationFrameLessThanEqual(FrameType type, double value) : mType(type), mValue(value)
      {
      }

      inline bool operator()(const AnimationFrame &lhs) const
      {
         switch (mType)
         {
         case FRAME_ID:
            return (lhs.mFrameNumber <= mValue);
            break;
         case FRAME_TIME:
            return (lhs.mTime <= mValue);
            break;
         default:
            break;
         };
         return false;
      }
   private:
      FrameType mType;
      double mValue;
   };

   class AnimationFrameGreaterThan
   {
   public:
      AnimationFrameGreaterThan(FrameType type, double value) : mType(type), mValue(value)
      {
      }

      inline bool operator()(const AnimationFrame &lhs) const
      {
         switch (mType)
         {
         case FRAME_ID:
            return (lhs.mFrameNumber > mValue);
            break;
         case FRAME_TIME:
            return (lhs.mTime > mValue);
            break;
         default:
            break;
         };
         return false;
      }
   private:
      FrameType mType;
      double mValue;
   };

}

AnimationImp::AnimationImp(FrameType frameType, const string& id) :
   SessionItemImp(id),
   mFrameType(frameType),
   mCurrentFrameIter(mFrames.begin())
{
}

AnimationImp::~AnimationImp()
{
}

const string& AnimationImp::getObjectType() const
{
   static string type = "AnimationImp";
   return type;
}

bool AnimationImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Animation"))
   {
      return true;
   }

   return false;
}

bool AnimationImp::attach(const std::string &signal, const Slot &slot)
{
   bool attached = SubjectImp::attach(signal, slot);
   emit objectAttached();
   return attached;
}

bool AnimationImp::detach(const std::string &signal, const Slot &slot)
{
   bool detached = SubjectImp::detach(signal, slot);
   emit objectDetached();
   return detached;
}

void AnimationImp::setName(const string& name)
{
   if (name != getName())
   {
      SessionItemImp::setName(name);
      emit renamed(QString::fromStdString(name));
      notify(SIGNAL_NAME(Animation, Renamed), boost::any(name));
   }
}

FrameType AnimationImp::getFrameType() const
{
   return mFrameType;
}

void AnimationImp::setFrames(const vector<AnimationFrame>& frames)
{
   if (frames != mFrames)
   {
      // Set and sort the frame data
      mFrames = frames;
      std::sort(mFrames.begin(), mFrames.end());

      mCurrentFrameIter = mFrames.end();
      setCurrentFrame(mFrames.begin());

      // Notify of changes
      emit framesChanged(mFrames);
      notify(SIGNAL_NAME(Animation, FramesChanged), boost::any(frames));
   }
}

const vector<AnimationFrame>& AnimationImp::getFrames() const
{
   return mFrames;
}

unsigned int AnimationImp::getNumFrames() const
{
   return (mFrames.size());
}

bool AnimationImp::hasFrame(const AnimationFrame* pFrame) const
{
   if (pFrame == NULL)
   {
      return false;
   }

   return binary_search(mFrames.begin(), mFrames.end(), *pFrame);
}

void AnimationImp::setCurrentFrame(const AnimationFrame* pFrame)
{
   VERIFYNRV(pFrame != NULL);
   setCurrentFrame(find(mFrames.begin(), mFrames.end(), *pFrame));
}

void AnimationImp::setCurrentFrame(FrameIterator frameIter)
{
   if (frameIter != mCurrentFrameIter)
   {
      mCurrentFrameIter = frameIter;
      notify(SIGNAL_NAME(Animation, FrameChanged), boost::any(const_cast<AnimationFrame*>(getCurrentFrame())));
   }
}

void AnimationImp::setCurrentFrame(double frameValue)
{
   FrameIterator frameIter = mFrames.end();

   // Do not set a valid current frame if the new value is greater than the stop value
   if (frameValue <= getStopValue() && frameValue >= getStartValue())
   {
      double currentValue = getFrameValue(getCurrentFrame());

      if (frameValue == currentValue)
      {
         frameIter = mCurrentFrameIter;
      }
      // Find the frame that is less than or equal to the given value
      else if (frameValue > currentValue)
      {
         FrameIterator currentIter = mCurrentFrameIter;
         if (currentIter == mFrames.end())
         {
            currentIter = mFrames.begin();
         }
         frameIter = find_if(currentIter, mFrames.end(),
            ::AnimationFrameGreaterThan(mFrameType, frameValue));
         VERIFYNRV(frameIter != mFrames.begin());
         --frameIter;
      }
      else
      {
         // Initializing a reverse iterator from a regular iterator so
         // the search starts from one frame before the current frame.
         vector<AnimationFrame>::reverse_iterator currentIter(mCurrentFrameIter);
         vector<AnimationFrame>::reverse_iterator foundIter = 
            find_if(currentIter, mFrames.rend(), 
            ::AnimationFrameLessThanEqual(mFrameType, frameValue));
         
         if (foundIter != mFrames.rend())
         {
            ++foundIter;
            frameIter = foundIter.base();
         }
      }
   }

   // Set the next frame as current
   setCurrentFrame(frameIter);
}
const AnimationFrame* AnimationImp::getCurrentFrame() const
{
   if (mCurrentFrameIter != mFrames.end())
   {
      return &*mCurrentFrameIter;
   }
   return NULL;
}

double AnimationImp::getStartValue() const
{
   double firstValue = -1.0;
   if (mFrames.empty() == false)
   {
      const AnimationFrame* pFrame = &(mFrames.front());
      firstValue = getFrameValue(pFrame);
   }

   return firstValue;
}

double AnimationImp::getStopValue() const
{
   double lastValue = -1.0;
   if (mFrames.empty() == false)
   {
      const AnimationFrame* pFrame = &(mFrames.back());
      lastValue = getFrameValue(pFrame);
   }

   return lastValue;
}

double AnimationImp::getFrameValue(const AnimationFrame* pFrame) const
{
   double frameValue = -1.0;
   if (pFrame != NULL)
   {
      if (mFrameType == FRAME_ID)
      {
         frameValue = pFrame->mFrameNumber;
      }
      else if (mFrameType == FRAME_TIME)
      {
         frameValue = pFrame->mTime;
      }
   }

   return frameValue;
}

double AnimationImp::getNextFrameValue(AnimationState direction, size_t offset) const
{
   if (mCurrentFrameIter == mFrames.end())
   {
      return -1;
   }
      
   vector<AnimationFrame>::const_iterator frame = mFrames.begin();
   if (direction == STOP || direction == PAUSE_FORWARD || direction == PAUSE_BACKWARD)
   {
      frame = mCurrentFrameIter;
   }
   else if (direction == PLAY_FORWARD)
   {
      frame = mCurrentFrameIter;
      double prevValue = -1.0;
      for (size_t i = 0; frame != mFrames.end(); ++i, ++frame)
      {
         double curValue = getFrameValue(&*frame);
         if (curValue == prevValue) // don't count duplicate frames
         {
            ++offset;
         }
         if (i >= offset)
         {
            break;
         }
         prevValue = curValue;
      }
   }
   else
   {
      frame = mCurrentFrameIter;
      while (frame != mFrames.begin() && offset > 0)
      {
         double prevValue = getFrameValue (&*frame);
         --frame;
         double curValue = getFrameValue (&*frame);
         if (prevValue != curValue) // don't count duplicate frames
         {
            --offset;
         }
      }

      if (offset != 0)
      {
         frame = mFrames.end();
      }
   }

   const AnimationFrame *pFrame = NULL;
   if (frame != mFrames.end())
   {
      pFrame = &*frame;
   }

   return getFrameValue(pFrame);
}

QString AnimationImp::frameToQString(double value, FrameType frameType, unsigned int count)
{
   QString frameString;
   if (frameType == FRAME_ID)
   {
      if (count > 0)
      {
         frameString.sprintf("Frame: %d/%d", static_cast<unsigned int> (value + 1.0), count);
      }
      else
      {
         frameString.sprintf("Frame: %d", static_cast<unsigned int> (value + 1.0));
      }
   }
   else
   {
      double seconds = floor(value);
      int milliseconds = static_cast<int> ((value - seconds) * 1000.0);

      QDateTime dateTime;
      dateTime.setTime_t(static_cast<unsigned int> (seconds));
      dateTime.setTime(dateTime.time().addMSecs(milliseconds));
      dateTime = dateTime.toUTC();

      frameString = dateTime.toString("yyyy/MM/dd hh:mm:ss.zzz") + "Z";
   }

   return frameString;
}

QString AnimationImp::frameToQString(const AnimationFrame* pFrame, FrameType frameType, unsigned int count)
{
  if (pFrame == NULL)
  {
     return QString();
  }
  else if (frameType == FRAME_ID)
  {
     return frameToQString(pFrame->mFrameNumber, frameType, count);
  }
  else
  {
     return frameToQString(pFrame->mTime, frameType, count);
  }
}

QString AnimationImp::frameToQString(const AnimationFrame* pFrame, unsigned int count)
{
   if (pFrame == NULL)
   {
      return QString();
   }
   else if (pFrame->mTime < 0)
   {
      return frameToQString(pFrame, FRAME_ID, count);
   }
   else
   {
      return frameToQString(pFrame, FRAME_TIME, count);
   }
}

bool AnimationImp::serialize(SessionItemSerializer& serializer) const
{
   XMLWriter xml("Animation");

   if(!SessionItemImp::toXml(&xml))
   {
      return false;
   }

   VERIFY(mFrameType.isValid());

   xml.addAttr("frametype", mFrameType);
   const AnimationFrame* pFrame = getCurrentFrame();
   if (pFrame != NULL)
   {
      switch (mFrameType)
      {
      case FRAME_ID:
         xml.addAttr("currentFrameID", pFrame->mFrameNumber);
         break;

      case FRAME_TIME:
         xml.addAttr("currentFrameTime", pFrame->mTime);
         break;

      default:
         VERIFY_MSG(false, "Unsupported frame type - animation will not be saved");
         break;
      }

   }
   for(vector<AnimationFrame>::const_iterator frame = mFrames.begin(); frame != mFrames.end(); ++frame)
   {
      xml.pushAddPoint(xml.addElement("frame"));
      xml.addAttr("name", frame->mName);
      xml.addAttr("number", frame->mFrameNumber);
      xml.addAttr("time", frame->mTime);
      xml.popAddPoint();
   }

   return serializer.serialize(xml);
}

bool AnimationImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement *pRoot = deserializer.deserialize(reader, "Animation");
   if(pRoot == NULL || !SessionItemImp::fromXml(pRoot, XmlBase::VERSION))
   {
      return false;
   }
   mFrameType = StringUtilities::fromXmlString<FrameType>(
      A(pRoot->getAttribute(X("frametype"))));

   VERIFY(mFrameType.isValid());

   for(DOMNode *pNode = pRoot->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(),X("DisplayText")))
      {
         setDisplayText(A(pNode->getTextContent()));
      }
      else if (XMLString::equals(pNode->getNodeName(),X("frame")))
      {
         DOMElement *pElement(static_cast<DOMElement*>(pNode));
         string name = A(pElement->getAttribute(X("name")));
         unsigned int number = StringUtilities::fromXmlString<unsigned int>(
            A(pElement->getAttribute(X("number"))));
         double time = StringUtilities::fromXmlString<double>(
            A(pElement->getAttribute(X("time"))));
         mFrames.push_back(AnimationFrame(name, number, time));
      }
   }

   mCurrentFrameIter = mFrames.end();
   switch(mFrameType)
   {
   case FRAME_ID:
      {
         unsigned int currentID(0);
         if (pRoot->hasAttribute(X("currentFrameID")))
         {
            currentID = StringUtilities::fromXmlString<unsigned int>(
               A(pRoot->getAttribute(X("currentFrameID"))));
            setCurrentFrame(currentID);
         }
         else
         {
            setCurrentFrame(&mFrames.front());
            VERIFY_MSG(false,"Problem occurred while restoring the current frame - Frame ID was not found");
         }
         break;
      }

   case FRAME_TIME:
      {
         double currentTime(0.0);
         if (pRoot->hasAttribute(X("currentFrameTime")))
         {
            currentTime = StringUtilities::fromXmlString<double>(
               A(pRoot->getAttribute(X("currentFrameTime"))));
            setCurrentFrame(currentTime);
         }
         else
         {
            setCurrentFrame(&mFrames.front());
            VERIFY_MSG(false,"Problem occurred while restoring the current frame - Frame Time was not found");
         }
         break;
      }

   default:
      VERIFY_MSG(false, "Unsupported frame type - animation will not be loaded");
      break;
   }

   return true;
}
