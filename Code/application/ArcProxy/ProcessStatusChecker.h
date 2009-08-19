/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROCESSSTATUSCHECKER_H
#define PROCESSSTATUSCHECKER_H

#include <QtCore/QObject>

/**
 * This class checks the status of a given process, and emits a signal
 * when that process is no longer running.
 */
class ProcessStatusChecker : public QObject
{
   Q_OBJECT

public:
   ProcessStatusChecker(QObject *pParent, long pid, unsigned int msec);
   ~ProcessStatusChecker();

private slots:
   void timeout();

signals:
   void processExited();

private:
   long mPid;
};

#endif