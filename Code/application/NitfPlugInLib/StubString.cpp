/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#ifdef _MSC_VER
//required in Visual C++ 2010 and later because std::string is now header only
//therefore these symbols are not being exported from OssimString and the ossim.dll
//See Microsoft site at:
//http://connect.microsoft.com/VisualStudio/feedback/details/586959/std-string-npos-unresolved-external-symbol-when-optimization-o2-o1-or-ox-enabled
#include <string>
template std::string::size_type std::string::npos;
template std::wstring::size_type std::wstring::npos;
#endif