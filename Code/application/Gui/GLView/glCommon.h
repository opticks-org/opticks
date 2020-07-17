/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GLCOMMON_H
#define GLCOMMON_H

#include "AppConfig.h"

#include <GL/glew.h>

#if defined(WIN_API)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#if defined(WIN_API)
#undef GL_GLEXT_PROTOTYPES
#include <GL/wglew.h>
#endif

#include "GlContextSave.h"
#include "GlTextureResource.h"

#endif
