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
// win_syscon.h
#include "../client/client.h"

#pragma amiga-align

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/window.h>
#include <proto/listbrowser.h>
#include <proto/texteditor.h>
#include <proto/string.h>
#include <proto/layout.h>

#include <classes/window.h>
#include <gadgets/listbrowser.h>
#include <gadgets/string.h>
#include <gadgets/layout.h>
#include <gadgets/texteditor.h>
#include <reaction/reaction_macros.h>

#pragma default-align

struct Window *con_window = 0;
Object *con_obj_window = 0;
Object *con_obj_list = 0;
Object *con_obj_input = 0;

int con_vislevel = 1;

#define CONSOLE_MAXCHARS 32768
char con_buffer[CONSOLE_MAXCHARS] = "Quake III Arena";
int con_bufferpos = 0;
int con_bufferend = 0;

struct Hook con_RenderHook;

enum GIDS
{
	GID_WINDOW = 1,
	GID_LIST,
	GID_INPUT
};

	
void Sys_CreateConsole( void )
{
	#if 0
	ULONG width = 400, height = 400;
	struct Screen *scr = Intuition->LockPubScreen(NULL);
	
	if (scr)
	{
		Intuition->GetScreenAttrs(scr,SA_Width,	&width, SA_Height, &height,TAG_DONE);
		Intuition->UnlockPubScreen(NULL, scr);
	}
	
	if (con_obj_window == 0)
	{
		con_obj_window = WindowObject,
			WA_Title,					"Quake 3 Console",
			WA_DragBar,					TRUE,
			WA_SmartRefresh,				TRUE,
			WA_CloseGadget,					TRUE,
			WA_SizeGadget,					TRUE,
			WA_DepthGadget,					TRUE,
			WA_Width,					640,
			WA_Height,					480, 
			WINDOW_Position,				WPOS_CENTERSCREEN,
			WINDOW_ParentGroup,				VLayoutObject, 
				LAYOUT_AddChild,		 	HGroupObject,
					LAYOUT_SpaceOuter,		TRUE,
					LAYOUT_SpaceInner,		TRUE,
					LAYOUT_BevelStyle,		BVS_GROUP,
					LAYOUT_Label,			"Console log",
					//LAYOUT_AddChild,		con_obj_list = TextEditorObject,GA_ID,GID_LIST,GA_ReadOnly,TRUE, End, /* SpaceObject */
					CHILD_WeightedHeight,		0,
				End, /* VGroupObject */
				
				LAYOUT_AddChild,			con_obj_input = StringObject,GA_ID,GID_INPUT,
				End, /* StringObject */
				CHILD_WeightedHeight, 		0,
				End,	/* VLayoutObject */

			End; /* Window */
	}
	
	if (con_obj_window == 0)
		return;
	
	con_window = RA_OpenWindow(con_obj_window);	
	#endif

}

void Sys_DestroyConsole( void )
{
	#if 0
	if (!con_obj_window)
		return;
		
	RA_CloseWindow(con_obj_window);
	Intuition->DisposeObject(con_obj_window);

	con_obj_window = 0;
	con_window = 0;
	#endif
}

char *Sys_ConsoleInput(void)
{
	return NULL;
}

void Sys_ShowConsole( int visLevel, qboolean quitOnClose )
{
	if (visLevel == con_vislevel || !con_obj_window)
		return;
		
	con_vislevel = visLevel;
	
	if (!con_obj_window)
		return;
		
	switch (visLevel)
	{
		case 0:
			//RA_CloseWindow(con_obj_window);
			con_window = 0;
			break;
		case 1:
		case 2:
			//con_window = RA_OpenWindow(con_obj_window);			
			break;
	}			
}

void Conbuf_AppendText( const char *pMsg )
{
	if (!con_obj_window)
		return;
}
