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

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../renderer/tr_public.h"
#include "../client/client.h"

#include "amiga_local.h"

#pragma amiga-align

#include <devices/timer.h>
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <proto/intuition.h>
#include <proto/keymap.h>

#pragma default-align

static cvar_t *in_mouse	 = NULL;
cvar_t *in_nograb        = NULL; 
cvar_t *in_joystick      = NULL;
cvar_t *in_joystickDebug = NULL;
cvar_t *joy_threshold    = NULL;

extern cvar_t *r_fullscreen;

qboolean mouse_avail;
qboolean mouse_active = qtrue;
int mx = 0, my = 0;

struct Library *KeymapBase = 0;
//struct KeymapIFace *IKeymap = 0;

void IN_ProcessEvents(void);

void IN_ActivateMouse( void ) 
{
	if (!mouse_avail)
		return;
		
	if (!mouse_active)
	{
		install_grabs();
			
		mouse_active = qtrue;
	}
}

void IN_DeactivateMouse( void ) 
{
	if (!mouse_avail)
		return;
		
	if (mouse_active)
	{
		uninstall_grabs();
	}

	mouse_active = qfalse;
}


void IN_Frame (void) 
{
	// IN_JoyMove(); // FIXME: disable if on desktop?
	IN_ActivateMouse();

	IN_ProcessEvents( );
}

void IN_Init(void) 
{
	if (!KeymapBase)
		KeymapBase = OpenLibrary("keymap.library", 0);
		
	//if (KeymapBase && !IKeymap)
		//IKeymap = (struct KeymapIFace *)GetInterface(KeymapBase, "main", 1, NULL);
		
	Com_Printf ("\n------- Input Initialization -------\n");

	// mouse variables
	in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);

	// developer feature, allows to break without loosing mouse pointer
	in_nograb = Cvar_Get ("in_nograb", "0", 0);

	in_joystick = Cvar_Get ("in_joystick", "0", CVAR_ARCHIVE|CVAR_LATCH);
  
	in_joystickDebug = Cvar_Get ("in_debugjoystick", "0", CVAR_TEMP);
	joy_threshold = Cvar_Get ("joy_threshold", "0.15", CVAR_ARCHIVE); // FIXME: in_joythreshold

	if (in_mouse->value)
		mouse_avail = qtrue;

	else
		mouse_avail = qfalse;

	//IN_StartupJoystick( ); // bk001130 - from cvs1.17 (mkv)

	Com_Printf ("------------------------------------\n");
}

void IN_Shutdown(void)
{
	mouse_avail = qfalse;
	
	/*
	if (IKeymap)
	{
		//DropInterface((struct Interface *)IKeymap);
		IKeymap = 0;
	}
	*/

	if (KeymapBase)
	{
		CloseLibrary(KeymapBase);
		KeymapBase = 0;
	}
}

static unsigned char scantokey[128] =
	{
	'`','1','2','3','4','5','6','7',                                 // 7
	'8','9','0','-','=','\\',0,K_INS,                                // f
	'q','w','e','r','t','y','u','i',                                 // 17
	'o','p','[',']',0,K_END,K_KP_DOWNARROW,K_PGDN,                   // 1f
	'a','s','d','f','g','h','j','k',                                 // 27
	'l',';','\'',0,0,K_KP_LEFTARROW,K_KP_5,K_KP_RIGHTARROW,          // 2f
	'<','z','x','c','v','b','n','m',                                 // 37
	',','.','/',0,K_KP_DEL,K_HOME,K_KP_UPARROW,K_PGUP,        	 // 3f
	K_SPACE,K_BACKSPACE,K_TAB,K_KP_ENTER,K_ENTER,K_ESCAPE,K_DEL,0,   // 47
	0,0,K_KP_MINUS,0,K_UPARROW,K_DOWNARROW,K_RIGHTARROW,K_LEFTARROW, // 4f
	K_F1,K_F2,K_F3,K_F4,K_F5,K_F6,K_F7,K_F8,                         // 57
	K_F9,K_F10,0,0,0,0,0,K_F11,                                      // 5f
	K_SHIFT,K_SHIFT,0,K_CTRL,K_ALT,K_ALT,0,0,               	 // 67
	0,0,0,0,0,0,0,0,                                                 // 6f
	0,0,0,0,0,0,0,0,                                                 // 77
	0,0,0,0,0,0,0,0                                                  // 7f
};

static int XLateKey(struct IntuiMessage *ev)
{
	//Com_Printf("Xlate %d\n", ev->Code);
	return scantokey[ev->Code&0x7f];
}

static qboolean keyDown(UWORD /*uint16*/ code)
{
	if (code & IECODE_UP_PREFIX)
		return qfalse;
	
	return qtrue;
}


//void Sys_HandleEvents(void)
void IN_ProcessEvents(void)
{
	struct IntuiMessage *imsg;
	//uint32 msgTime;
	//int xlated;
	struct InputEvent ie;
	//int16 res;
	WORD res;
	char buf[20];

	if (!Sys_EventPort)
		return;

	while ((imsg = (struct IntuiMessage *)GetMsg(Sys_EventPort)))
	{
		const ULONG msgTime = 0;//Sys_Milliseconds();
		
		switch (imsg->Class)
		{
			case IDCMP_RAWKEY:
			{
				int key = XLateKey(imsg);

				//if (IKeymap) // Cowcat
				{
					//Com_Printf ("key encoded %d %d\n", imsg->Code, imsg->Qualifier);

					ie.ie_Class = IECLASS_RAWKEY;
					ie.ie_SubClass = 0;
					ie.ie_Code = imsg->Code;
					ie.ie_Qualifier = imsg->Qualifier;
					ie.ie_EventAddress = 0;

					if (key == '`')
					{
						key = K_CONSOLE;
						res = 0;
					}

					else
						res = MapRawKey(&ie, buf, 20, 0);
				}

				Com_QueueEvent(msgTime, SE_KEY, key, keyDown(imsg->Code), 0, NULL);

				if (res == 1)
				{
					Com_QueueEvent(msgTime, SE_CHAR, buf[0], 0, 0, NULL);
				}

			}
				
			break;
			
			case IDCMP_MOUSEMOVE:

				if (mouse_active)
				{
					mx = imsg->MouseX;
					my = imsg->MouseY;
					Com_QueueEvent(msgTime, SE_MOUSE, mx, my, 0, NULL);
				}

				break;

			/*
			case IDCMP_EXTENDEDMOUSE:
			// FIXME: Add additional mouse buttons
				if (imsg->Code & IMSGCODE_INTUIWHEELDATA)
				{
					struct IntuiWheelData *iwd = (struct IntuiWheelData *)imsg->IAddress;
							
					if (iwd->WheelY > 0)
					{
						Com_QueueEvent(msgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL);
						Com_QueueEvent(msgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL);
					}
					else if (iwd->WheelY < 0)
					{
						Com_QueueEvent(msgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL);
						Com_QueueEvent(msgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL);
					}
				}
				break;
			*/

			case IDCMP_MOUSEBUTTONS:

				switch (imsg->Code & ~IECODE_UP_PREFIX)
				{
					case IECODE_LBUTTON:
						Com_QueueEvent(msgTime, SE_KEY, K_MOUSE1, keyDown(imsg->Code),0, NULL);
						break;

					case IECODE_RBUTTON:
						Com_QueueEvent(msgTime, SE_KEY, K_MOUSE2, keyDown(imsg->Code),0, NULL);
						break;

					case IECODE_MBUTTON:
						Com_QueueEvent(msgTime, SE_KEY, K_MOUSE3, keyDown(imsg->Code),0, NULL);
						break;
				}
		}

		if (Sys_EventPort)
			ReplyMsg((struct Message *)imsg);

		else	
			return;
		
	}
}

