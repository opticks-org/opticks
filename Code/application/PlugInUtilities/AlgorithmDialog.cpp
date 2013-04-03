/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AlgorithmDialog.h"

AlgorithmDialog::AlgorithmDialog(QWidget* pParent) :
   QDialog(pParent),
   mpEventLoop(new QEventLoop(this))
{
   setModal(false);
}

AlgorithmDialog::~AlgorithmDialog()
{
   if (mpEventLoop->isRunning() == true)
   {
      mpEventLoop->exit(QDialog::Rejected);
   }
}

int AlgorithmDialog::enterLoop()
{
   if (mpEventLoop->isRunning() == false)
   {
      return mpEventLoop->exec();
   }

   return QDialog::Rejected;
}

void AlgorithmDialog::accept()
{
   QDialog::accept();

   if (mpEventLoop->isRunning() == true)
   {
      mpEventLoop->exit(QDialog::Accepted);
   }
}

void AlgorithmDialog::reject()
{
   QDialog::reject();

   if (mpEventLoop->isRunning() == true)
   {
      mpEventLoop->exit(QDialog::Rejected);
   }
}
