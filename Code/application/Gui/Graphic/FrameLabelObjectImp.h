/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FRAMELABELOBJECTIMP_H
#define FRAMELABELOBJECTIMP_H

#include "AnimationController.h"
#include "AttachmentPtr.h"
#include "LayerList.h"
#include "TextObjectImp.h"
#include "View.h"

#include <string>
#include <vector>

class Animation;
class RasterLayer;
class Subject;

class FrameLabelObjectImp : public TextObjectImp
{
public:
   FrameLabelObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~FrameLabelObjectImp();

   void setAutoMode(bool autoMode);
   bool getAutoMode() const;
   bool processMousePress(LocationType screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   void setAnimations(View* pView);
   void setAnimations(const std::vector<Animation*> &animations);

   const std::vector<Animation*> &getAnimations() const;
   void insertAnimations(const std::vector<Animation*> &animations);
   void insertAnimation(Animation* pAnimation);
   void eraseAnimation(Animation* pAnimation);
   void clearAnimations();

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void setLayer(GraphicLayer* pLayer);
   void updateGeo();
   bool replicateObject(const GraphicObject* pObject);

protected:
   void frameChanged(Subject &subject, const std::string &signal, const boost::any &value);
   void animationAdded(Subject &subject, const std::string &signal, const boost::any &value);
   void animationRemoved(Subject &subject, const std::string &signal, const boost::any &value);
   void animationDeleted(Subject &subject, const std::string &signal, const boost::any &value);
   void layerDeleted(Subject &subject, const std::string &signal, const boost::any &value);

   void updateAnimations(Subject &subject, const std::string &signal, const boost::any &value);

private:
   FrameLabelObjectImp(const FrameLabelObjectImp& rhs);
   FrameLabelObjectImp& operator=(const FrameLabelObjectImp& rhs);
   bool mAutoMode;
   bool mLocked;
   AttachmentPtr<View> mpView;
   AttachmentPtr<LayerList> mpLayerList;
   std::vector<RasterLayer*> mLayers;
   AttachmentPtr<AnimationController> mpAnimationController;
   std::vector<Animation*> mAnimations;

   void updateText();
   void reset();
   void clearLayers();
   void eraseLayer(RasterLayer* pLayer);
   void updateAnimationList(bool force = false);
   void setLocked(bool locked);
   bool getLocked() const;
};

#define FRAMELABELOBJECTADAPTEREXTENSION_CLASSES \
   TEXTOBJECTADAPTEREXTENSION_CLASSES

#define FRAMELABELOBJECTADAPTER_METHODS(impClass) \
   TEXTOBJECTADAPTER_METHODS(impClass) \
   const std::vector<Animation*> &getAnimations() const \
   { \
      return impClass::getAnimations(); \
   } \
   void setAnimations(const std::vector<Animation*> &animations) \
   { \
      impClass::setAnimations(animations); \
   } \
   void insertAnimation(Animation* animation) \
   { \
      impClass::insertAnimation(animation); \
   } \
   void eraseAnimation(Animation* animation) \
   { \
      impClass::eraseAnimation(animation); \
   } \
   void setAutoMode(bool autoMode) \
   { \
      impClass::setAutoMode(autoMode); \
   } \
   bool getAutoMode() const \
   { \
      return impClass::getAutoMode(); \
   } 

#endif
