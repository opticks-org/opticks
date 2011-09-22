/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QWidget>

#include <string>
#include <list>

class Progress;
class Subject;
class QTimer;

namespace boost
{
   class any;
}

class SplashScreen : public QWidget
{
   Q_OBJECT

public:
   SplashScreen(Progress* pProgress);
   ~SplashScreen();

   void setSplashImages(const std::list<std::string>& imagePaths);
   void update(Subject &subject, const std::string &signal, const boost::any &v);
   bool canClose();

protected slots:
   void rotateImage();

protected:
   bool event(QEvent* pEvent);
   void mousePressEvent(QMouseEvent* pEvent);

private:
   Progress* mpProgress;
   QLabel* mpProgressLabel;
   QProgressBar* mpProgressBar;
   QLabel* mpPixmapLabel;
   QLabel* mpReleaseInfo;
   std::list<std::string> mImagePaths;
   QTimer* mpRotateImageTimer;
};

#endif
