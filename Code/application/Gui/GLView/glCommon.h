/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GLCOMMON_H
#define GLCOMMON_H

#include "AppConfig.h"

#if defined(CG_SUPPORTED)
#include <GL/glew.h>
#endif

#if defined(WIN_API)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

// Cg header files
#if defined(CG_SUPPORTED)
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#if defined(WIN_API)
#undef GL_GLEXT_PROTOTYPES
#include <GL/wglew.h>
#endif
#endif

#include "GlContextSave.h"

#endif
