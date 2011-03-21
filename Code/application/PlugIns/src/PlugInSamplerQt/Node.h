/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NODE_H
#define NODE_H

#include <vector>
#include "ColorType.h"

namespace PlugInSamplerQt
{
/**
 * Defines the type of a single color node
 */
typedef std::pair<double, ColorType> Node;
};

#endif
