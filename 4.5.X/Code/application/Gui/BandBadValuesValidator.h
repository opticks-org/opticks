/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef BANDBADVALUESVALIDATOR_H
#define BANDBADVALUESVALIDATOR_H

#include <QtGui/QValidator>

#include <vector>

class BandBadValuesValidator : public QValidator
{
public:
   BandBadValuesValidator(QObject* parent);
   ~BandBadValuesValidator();

   QValidator::State validate(QString& input, int& pos) const;
   static QString convertVectorToString(const std::vector<int>& vec);
   static const std::vector<int> convertStringToVector(QString& input, bool& result);
};

#endif
