/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FUSION_TOOLS_H
#define FUSION_TOOLS_H

class DataFusionTools
{
public:
   // used by Poly2D/PolyWarp to access plug-in objects
   static inline void setAbortFlag(bool bFlag) { smbAbortFlag = bFlag; }
   static inline bool getAbortFlag() { return smbAbortFlag; }

private:
   static bool smbAbortFlag;
};

#endif
