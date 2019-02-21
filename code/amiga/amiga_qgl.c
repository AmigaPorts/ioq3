/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <float.h>
#include "../renderer/tr_local.h"

#include <proto/exec.h>
#include <mgl/gl.h>

#if 0
//struct MiniGLIFace *IMiniGL = 0;
//struct Library *MiniGLBase = 0;

void MiniGL_Init(void)
{
	/*
	// Open MiniGL.library and get the interface
	MiniGLBase = IExec->OpenLibrary("minigl.library", 0);
	if (!MiniGLBase)
		ri.Error( ERR_FATAL, "GL_StartOpenGL() - could not load OpenGL subsystem\n");

	IMiniGL = (struct MiniGLIFace *)IExec->GetInterface(MiniGLBase, "main", 1, NULL);
	if (!IMiniGL)
	{
		IExec->CloseLibrary(MiniGLBase);
		ri.Error(ERR_FATAL, "GL_StartOpenGL() - could not load OpenGL subsystem\n");
	}
	*/
}

#endif

void MiniGL_Term(void)
{
	/*
	if (IMiniGL)
	{
		IExec->DropInterface((struct Interface *)IMiniGL);
		IExec->CloseLibrary(MiniGLBase);
		
		IMiniGL = 0;
		MiniGLBase = 0;
	}
	*/
}

void QGL_Shutdown(void)
{
	//MiniGL_Term();
	MGLTerm();
}


void QGL_Init(const char *name)
{
	//MiniGL_Init();
	MGLInit();
}

