/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Slot.h"
#include "TiePointTableModel.h"

using namespace std;

TiePointTableModel::TiePointTableModel(QObject* pParent) :
   QAbstractTableModel(pParent),
   mpTiePointList(NULL),
   mRows(0)
{
}

TiePointTableModel::~TiePointTableModel()
{
   setTiePointList(NULL);
}

QVariant TiePointTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (role != Qt::DisplayRole)
   {
      return QVariant();
   }

   if (orientation == Qt::Horizontal)
   {
      if (section == 0)
      {
         return QVariant("Ref X");
      }
      else if (section == 1)
      {
         return QVariant("Ref Y");
      }
      else if (section == 2)
      {
         return QVariant("Mission X");
      }
      else if (section == 3)
      {
         return QVariant("Mission Y");
      }
      else if (section == 4)
      {
         return QVariant("Confidence");
      }
      else if (section == 5)
      {
         return QVariant("Phi");
      }
   }
   else if (orientation == Qt::Vertical)
   {
      return QVariant(QString::number(section + 1));
   }

   return QVariant();
}

int TiePointTableModel::rowCount(const QModelIndex& parent) const
{
   return mRows;
}

int TiePointTableModel::columnCount(const QModelIndex& parent) const
{
   return 6;
}

Qt::ItemFlags TiePointTableModel::flags(const QModelIndex& index) const
{
   Qt::ItemFlags flag = QAbstractTableModel::flags(index);
   if (index.isValid() == true)
   {
      flag |= Qt::ItemIsEditable;
   }

   return flag;
}

bool TiePointTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
   if ((index.isValid() == false) || (role != Qt::EditRole) || (mpTiePointList == NULL))
   {
      return false;
   }

   int row = index.row();
   int column = index.column();

   vector<TiePoint> oldPoints = mpTiePointList->getTiePoints();
   vector<TiePoint> tiePoints = oldPoints;
   if (static_cast<int>(tiePoints.size()) <= row)
   {
      return false;
   }

   TiePoint& tiePoint = tiePoints.at(row);
   int intField = 0;
   float floatField = 0.0f;
   bool bSuccess = false;

   switch (column)
   {
      case 0:
         intField = value.toInt(&bSuccess);
         if (bSuccess == true)
         {
            if (tiePoint.mReferencePoint.mX != intField - 1)
            {
               tiePoint.mReferencePoint.mX = intField - 1;
            }
            else
            {
               bSuccess = false;
            }
         }
         break;

      case 1:
         intField = value.toInt(&bSuccess);
         if (bSuccess == true)
         {
            if (tiePoint.mReferencePoint.mY != intField - 1)
            {
               tiePoint.mReferencePoint.mY = intField - 1;
            }
            else
            {
               bSuccess = false;
            }
         }
         break;

      case 2:
         floatField = value.toString().toFloat(&bSuccess);
         if (bSuccess == true)
         {
            if (tiePoint.mMissionOffset.mX != floatField)
            {
               tiePoint.mMissionOffset.mX = floatField;
            }
            else
            {
               bSuccess = false;
            }
         }
         break;

      case 3:
         floatField = value.toString().toFloat(&bSuccess);
         if (bSuccess == true)
         {
            if (tiePoint.mMissionOffset.mY != floatField)
            {
               tiePoint.mMissionOffset.mY = floatField;
            }
            else
            {
               bSuccess = false;
            }
         }
         break;

      case 4:
         intField = value.toInt(&bSuccess);
         if (bSuccess == true)
         {
            if (tiePoint.mConfidence != intField)
            {
               tiePoint.mConfidence = intField;
            }
            else
            {
               bSuccess = false;
            }
         }
         break;

      case 5:
         intField = value.toInt(&bSuccess);
         if (bSuccess == true)
         {
            if (tiePoint.mPhi != intField)
            {
               tiePoint.mPhi = intField;
            }
            else
            {
               bSuccess = false;
            }
         }
         break;

      default:
         break;
   }

   if (bSuccess == true)
   {
      mpTiePointList->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointTableModel::tiePointListModified));
      mpTiePointList->adoptTiePoints(tiePoints);
      mpTiePointList->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointTableModel::tiePointListModified));

      const vector<TiePoint>& newPoints = mpTiePointList->getTiePoints();
      emit pointsModified(oldPoints, newPoints);
      emit dataChanged(index, index);
   }

   return bSuccess;
}

QVariant TiePointTableModel::data(const QModelIndex& index, int role) const
{
   if ((mpTiePointList == NULL) || (index.isValid() == false) || ((role != Qt::DisplayRole) && (role != Qt::EditRole)))
   {
      return QVariant();
   }

   int row = index.row();
   int column = index.column();

   const vector<TiePoint>& tiePoints = mpTiePointList->getTiePoints();
   if (row >= static_cast<int>(tiePoints.size()))
   {
      return QVariant();
   }

   TiePoint tiePoint = tiePoints[row];
   QString cellText;

   switch (column)
   {
      case 0:
         cellText = QString::number(tiePoint.mReferencePoint.mX + 1);
         break;

      case 1:
         cellText = QString::number(tiePoint.mReferencePoint.mY + 1);
         break;

      case 2:
         cellText = QString::number(tiePoint.mMissionOffset.mX);
         break;

      case 3:
         cellText = QString::number(tiePoint.mMissionOffset.mY);
         break;

      case 4:
         cellText = QString::number(tiePoint.mConfidence);
         break;

      case 5:
         cellText = QString::number(tiePoint.mPhi);
         break;

      default:
         break;
   }

   if (cellText.isEmpty() == false)
   {
      return QVariant(cellText);
   }

   return QVariant();
}

void TiePointTableModel::setTiePointList(TiePointList* pTiePointList)
{
   if (pTiePointList == mpTiePointList)
   {
      return;
   }

   clear();

   if (mpTiePointList != NULL)
   {
      mpTiePointList->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointTableModel::tiePointListModified));
      mpTiePointList->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointTableModel::tiePointListDeleted));
   }

   mpTiePointList = pTiePointList;

   if (mpTiePointList != NULL)
   {
      mpTiePointList->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &TiePointTableModel::tiePointListModified));
      mpTiePointList->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &TiePointTableModel::tiePointListDeleted));
   }

   addTiePoints();
}

void TiePointTableModel::tiePointListModified(Subject& subject, const string& signal, const boost::any& value)
{
   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(&subject);
   if (pTiePointList == mpTiePointList)
   {
      clear();
      addTiePoints();
   }
}

void TiePointTableModel::tiePointListDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   TiePointList* pTiePointList = dynamic_cast<TiePointList*>(&subject);
   if (pTiePointList == mpTiePointList)
   {
      setTiePointList(NULL);
   }
}

void TiePointTableModel::addTiePoints()
{
   mRows = 0;
   if (mpTiePointList != NULL)
   {
      const vector<TiePoint>& tiePoints = mpTiePointList->getTiePoints();
      mRows = static_cast<int>(tiePoints.size());
   }

   if (mRows > 0)
   {
      beginInsertRows(QModelIndex(), 0, mRows - 1);
      endInsertRows();
   }
}

void TiePointTableModel::clear()
{
   if (mRows > 0)
   {
      beginRemoveRows(QModelIndex(), 0, mRows - 1);
      endRemoveRows();
   }

   mRows = 0;
}
