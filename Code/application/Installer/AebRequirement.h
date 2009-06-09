/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AEBREQUIREMENT_H
#define AEBREQUIREMENT_H

#include "AebId.h"
#include "AebVersion.h"

class AebRequirement
{
public:
   AebRequirement();
   AebRequirement(AebId id, AebVersion min, AebVersion max);
   AebRequirement(const AebRequirement& other);
   bool isValid() const;
   AebId getId() const;
   AebVersion getMin() const;
   AebVersion getMax() const;
   bool meets(AebVersion version) const;
   bool operator==(const AebRequirement& other) const;
   bool operator!=(const AebRequirement& other) const;
   bool operator<(const AebRequirement& other) const;

private:
   AebId mId;
   AebVersion mMin;
   AebVersion mMax;
};

#endif