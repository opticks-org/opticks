/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Hdf4File.h"
#include "Hdf4Group.h"

using namespace std;

Hdf4File::Hdf4File(const string& name) :
   Hdf4Element(name),
   mpActiveDataset(NULL),
   mpRootGroup(new Hdf4Group("/"))
{
}

Hdf4File::~Hdf4File()
{
   delete mpRootGroup;
}

Hdf4Group* Hdf4File::getRootGroup() const
{
   return mpRootGroup;
}
