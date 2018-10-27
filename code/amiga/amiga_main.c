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

#define __USE_BASETYPE__

#ifdef __VBCC__
#pragma amiga-align
#elif defined(WARPUP)
#pragma pack(2)
#endif

#include <exec/exec.h>
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <devices/timer.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <proto/intuition.h>

#ifdef __VBCC__
#pragma default align
#elif defined (WARPUP)
#pragma pack()
#endif

#ifdef DLL
#include "dll.h"
#endif

//#include <devices/timer.h>
//#include <inline/timer_protos.h>

//#include <dlfunc.h>

#ifdef __VBCC__
#define CreateExtIO(p,s) CreateIORequest(p,s)
#define DeletePort(p) DeleteMsgPort(p)
#define DeleteExtIO(p) DeleteIORequest(p)
#endif

struct MsgPort *Sys_EventPort = 0;

//#if !defined(__GNUC__)
struct Library *SocketBase; // = 0;
//#endif

int	totalMsec, countMsec;

/*
#define MAX_QUED_EVENTS		256
#define MASK_QUED_EVENTS 	(MAX_QUED_EVENTS - 1)

sysEvent_t eventQue[MAX_QUED_EVENTS];
int eventHead = 0;
int eventTail = 0;
byte sys_packetReceived[MAX_MSGLEN];
*/

#define MEM_THRESHOLD 96*1024*1024

static qboolean consoleoutput = qfalse;

cvar_t *sys_nostdout;
static BPTR amiga_stdout;

qboolean Sys_LowPhysicalMemory() // It is always true - Cowcat
{
	return qtrue; 
	
	//ULONG avail = AvailMem(MEMF_FAST);
	//return (avail <= MEM_THRESHOLD) ? qtrue : qfalse;
}

#if 1

extern char *FS_BuildOSPath( const char *base, const char *game, const char *qpath );

//void *Sys_LoadDll( const char *name, char *fqpath, intptr_t(**entryPoint)(int, ...), intptr_t(*systemcalls)(intptr_t, ...) ) 
void *Sys_LoadDll( const char *name, char *fqpath, intptr_t(**entryPoint)(int, int, int, int ), intptr_t(*systemcalls)(intptr_t, ...) ) // Cowcat
//void *Sys_LoadDll( const char *name, char *fqpath, 
	//intptr_t(**entryPoint)(int, int, int, int, int, int, int, int, int, int, int, int, int ), intptr_t(*systemcalls)(intptr_t*) ) // Cowcat
{
#ifdef DLL

	char	fname[MAX_OSPATH];
	char	curpath[MAX_OSPATH];
	char	*basepath;
	char	*pwdpath;
	char	*gamedir;
	char	*cdpath;
	char	*fn;
	void	*libHandle;
	void	(*dllEntry)( intptr_t (*syscallptr)(intptr_t, ...) );
	//void	(*dllEntry)( intptr_t (*syscallptr)(intptr_t*) ); // new

	fname[0] = 0;
	
	getcwd(curpath, sizeof(curpath));
	
	#if 0 // Cowcat

	#ifdef __PPC__
	snprintf(fname, sizeof(fname), "%sppc.so", name);
	#else
	snprintf(fname, sizeof(fname), "%s68k.so", name);
	#endif

	#endif

	pwdpath = Sys_Cwd();
	basepath = Cvar_VariableString( "fs_basepath" );
	gamedir = Cvar_VariableString( "fs_game" );
	cdpath = Cvar_VariableString( "fs_cdpath" );

	fn = FS_BuildOSPath( pwdpath, gamedir, name ); // was fname - Cowcat
	Com_Printf( "Sys_LoadDll(%s)... \n", fn );
	libHandle = dllLoadLibrary(fn, name); // was fname - Cowcat

	if (!libHandle)
		return NULL;

	dllEntry = dllGetProcAddress(libHandle, "dllEntry");
	*entryPoint = dllGetProcAddress(libHandle, "vmMain");

	if (!*entryPoint || !dllEntry)
	{
		dllFreeLibrary(libHandle);
		return NULL;
	}
	
	dllEntry(systemcalls);

	if (libHandle)
		Q_strncpyz(fqpath, fn, MAX_QPATH);

	return libHandle;

#else

	return NULL;

#endif
}

#else

void *Sys_LoadDll( const char *name, char *fqpath, intptr_t(**entryPoint)(int, int, int, int ), intptr_t(*systemcalls)(intptr_t, ...) ) // Cowcat
{
#ifdef DLL

	char	fname[MAX_OSPATH];
	char	curpath[MAX_OSPATH];
	char	*basepath;
	char	*pwdpath;
	char	*gamedir;
	char	*cdpath;
	char	*fn;
	void	*libHandle;
	void	(*dllEntry)( intptr_t (*syscallptr)(intptr_t, ...) );
	//void	(*dllEntry)( intptr_t (*syscallptr)(intptr_t*) ); // new

	fname[0] = 0;
	
	getcwd(curpath, sizeof(curpath));
	
	#ifdef __PPC__
	snprintf(fname, sizeof(fname), "%sppc.so", name);
	#else
	snprintf(fname, sizeof(fname), "%s68k.so", name);
	#endif

	pwdpath = Sys_Cwd();
	basepath = Cvar_VariableString( "fs_basepath" );
	gamedir = Cvar_VariableString( "fs_game" );
	cdpath = Cvar_VariableString( "fs_cdpath" );

	fn = FS_BuildOSPath( pwdpath, gamedir, fname );
	Com_Printf( "Sys_LoadDll(%s)... \n", fn );
	libHandle = dllLoadLibrary(fn, fname);

	if (!libHandle)
		return NULL;

	dllEntry = dllGetProcAddress(libHandle, "dllEntry");
	*entryPoint = dllGetProcAddress(libHandle, "vmMain");

	if (!*entryPoint || !dllEntry)
	{
		dllFreeLibrary(libHandle);
		return NULL;
	}
	
	dllEntry(systemcalls);

	if (libHandle)
		Q_strncpyz(fqpath, fn, MAX_QPATH);

	return libHandle;

#else

	return NULL;

#endif
}

#endif

void Sys_UnloadDll(void *dllHandle)
{
	if (dllHandle)
	{
		#ifdef DLL
		dllFreeLibrary(dllHandle);
		#endif
	}
}
	
		  
void Sys_BeginProfiling( void ) 
{
}

#if 0 // not used now - Cowcat

struct timerequest *timerio;
struct MsgPort *timerport;
struct Library *TimerBase;

void Timer_Init(void)
{
	if (timerport = CreateMsgPort ())
	{
		if (timerio = (struct timerequest *) CreateExtIO(timerport,sizeof(struct timerequest)))
		{
			if (OpenDevice(TIMERNAME,UNIT_MICROHZ, (struct IORequest *)timerio,0) == 0)
			{
				#ifdef __GNUC__
				TimerBase = (struct Device *)timerio->tr_node.io_Device;
				#else
				TimerBase = (struct Library *)timerio->tr_node.io_Device;
				#endif
			}

			else
			{
				DeleteExtIO((struct IORequest *)timerio);
				DeletePort(timerport);
			}
		}

		else
			DeletePort(timerport);
	}

	if (!TimerBase)
		Sys_Error("Can't open timer.device");

	//TimerBase = (struct Library *)FindName(&SysBase->DeviceList,"timer.device");
}

void Timer_Term(void)
{
	if (TimerBase)
	{
		if (!CheckIO((struct IORequest *)timerio))
		{
			AbortIO((struct IORequest *)timerio);
			WaitIO((struct IORequest *)timerio);
		}

		CloseDevice((struct IORequest *)timerio);
		DeletePort(timerio->tr_node.io_Message.mn_ReplyPort);
		DeleteExtIO((struct IORequest *)timerio);
	}
}

#endif
		
void Sys_Exit(int ex)
{
	//Sys_DestroyConsole();
	//Timer_Term(); // not used now - Cowcat
	exit(ex);
}

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void QDECL Sys_Error( const char *error, ... )
{
	va_list		argptr;
	//char		text[4096];
	char		string[1024]; // new Cowcat

	va_start (argptr, error);
	//vsprintf (text, error, argptr);
	Q_vsnprintf (string, sizeof(string), error, argptr); // new Cowcat
	va_end (argptr);

	//CL_Shutdown(string, qtrue); // new Cowcat

	//Conbuf_AppendText( text ); // Cowcat
	//Sys_ShowConsole( 1, qtrue ); //

	//IExec->DebugPrintF("Sys_Error: %s\n", text);
	//fprintf(stderr, "Sys_Error: %s\n", text);
	fprintf(stderr, "Sys_Error: %s\n", string);

	Sys_Exit(1);
}	


/*
==============
Sys_Quit
==============
*/
void Sys_Quit( void ) 
{
	//CL_Shutdown(); // new - disabled Cowcat
//	IN_Shutdown();

	Sys_Exit(0);
}


/*
==============
Sys_Print
==============
*/
void Sys_Print( const char *msg ) 
{
	//IExec->DebugPrintF("%s", msg);
	//Conbuf_AppendText( msg );
	if(!consoleoutput)
		return;

	fputs(msg, stdout);
}



/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void ) 
{
  	IN_Shutdown();
  	IN_Init();
}



/*
================
Sys_Init

Called after the common systems (cvars, files, etc)
are initialized
================
*/

void Sys_Init( void )
{
	char *cpuidstr;
	
	Cmd_AddCommand ("in_restart", Sys_In_Restart_f);

	Cvar_Set("arch", "amigaos");
	
	/* Figure out CPU */

	#if 0 // Cowcat
	Cvar_Get("sys_cpustring", "detect", 0);
		
	Com_Printf("...detecting CPU, found ");
	//IExec->GetCPUInfoTags(GCIT_ModelString, &cpuidstr, TAG_DONE);
			
	Cvar_Set("sys_cpustring", cpuidstr);
	
	Com_Printf("%s\n", cpuidstr);
	
	Cvar_Set("username", Sys_GetCurrentUser());
	#endif

	// Cowcat
	sys_nostdout = Cvar_Get("sys_nostdout", "1", CVAR_ARCHIVE);

	if(!sys_nostdout->value)  // Cowcat
	{
		amiga_stdout = Output();
		consoleoutput = qtrue;
	}
	//

	IN_Init();
}

qboolean Sys_CheckCD( void ) 
{
  	return qtrue;
}


void Sys_InitStreamThread( void )
{
}

void Sys_ShutdownStreamThread( void )
{
}

void Sys_BeginStreamedFile( fileHandle_t f, int readAhead )
{
}

void Sys_EndStreamedFile( fileHandle_t f )
{
}

int Sys_StreamedRead( void *buffer, int size, int count, fileHandle_t f )
{
  	return FS_Read( buffer, size * count, f );
}

void Sys_StreamSeek( fileHandle_t f, int offset, int origin )
{
  	FS_Seek( f, offset, origin );
}

char *Sys_GetClipboardData(void)
{
  	return NULL;
}



/*
================
Sys_QueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
//void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr )
/*
void Com_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr )
{
	sysEvent_t	*ev;

	ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];
	
	if ( eventHead - eventTail >= MAX_QUED_EVENTS )
	{
		Com_Printf("Sys_QueEvent: overflow\n");

		// we are discarding an event, but don't leak memory
		if ( ev->evPtr ) {
			Z_Free( ev->evPtr );
		}

		eventTail++;
	}

	eventHead++;

	if ( time == 0 )
	{
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}


sysEvent_t Sys_GetEvent(void)
{
	sysEvent_t ev;
	char *s;
	msg_t netmsg;
	netadr_t adr;
	
	if (eventHead > eventTail) 
	{
		eventTail++;
		return eventQue [(eventTail - 1) &MASK_QUED_EVENTS ];
	}
	
	//Sys_HandleEvents();
	IN_ProcessEvents();

	s = Sys_ConsoleInput();

	if (s)
	{
		char *b;
		int len;
		
		len = strlen(s) + 1;
		b = Z_Malloc(len);
		strcpy (b, s);
		Com_QueEvent(0, SE_CONSOLE, 0, 0, len, b);
	}
	
	MSG_Init(&netmsg, sys_packetReceived, sizeof(sys_packetReceived));

	if (Sys_GetPacket(&adr, &netmsg))
	{
		netadr_t *buf;
		int len;
		
		len = sizeof( netadr_t ) + netmsg.cursize - netmsg.readcount;
		buf = Z_Malloc( len );
		*buf = adr;
		memcpy( buf+1, &netmsg.data[netmsg.readcount], netmsg.cursize - netmsg.readcount );
		Com_QueEvent( 0, SE_PACKET, 0, 0, len, buf );
	}

	// return if we have data
	if ( eventHead > eventTail ) 
	{
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}
	
	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = Sys_Milliseconds();

	return ev;
}	
*/

//static char __attribute__((used)) stackcookie[] = "$STACK:2000000";
unsigned long __stack = 0x2000000; // auto stack Cowcat

int main(int argc, char **argv)
{
	char 	*cmdline;
	int 	i, len;
	int	startTime, endTime;

	//Timer_Init(); // not used now - Cowcat
	
	//#if !defined(__GNUC__)
	if(SocketBase == NULL)
		SocketBase = OpenLibrary("bsdsocket.library",0L);
	//#endif

	//Sys_CreateConsole();

	Sys_Milliseconds();
	
	// merge the command line, this is kinda silly
	for (len = 1, i = 1; i < argc; i++)
		len += strlen(argv[i]) + 1;

	cmdline = malloc(len);
	*cmdline = 0;

	for (i = 1; i < argc; i++)
	{
		if (i > 1)
			strcat(cmdline, " ");

		strcat(cmdline, argv[i]);
	}

	//memset( &eventQue[0], 0, MAX_QUED_EVENTS*sizeof(sysEvent_t) ); 
	//memset( &sys_packetReceived[0], 0, MAX_MSGLEN*sizeof(byte) );

	Com_Init(cmdline);

	free(cmdline);
	cmdline = NULL;

	NET_Init();

	while( 1 ) 
	{
		startTime = Sys_Milliseconds();

		// make sure mouse and joystick are only called once a frame
		//IN_Frame(); // now in common.c - Com_Frame

		// run the game
		Com_Frame();

		endTime = Sys_Milliseconds();
		totalMsec += endTime - startTime;
		countMsec++;
	}

	return 0; 
}

#if 0 // not used now - Cowcat
void Sys_Sleep(int msec) // used in common.c/Com_Frame - never reached? - Cowcat
{
	// Just in case - Cowcat
	if( msec == 0)
		return;

	if( msec < 0 )
		msec = 10;
	//

	usleep(1000 * msec);
}
#endif

char *Sys_ConsoleInput(void) // Cowcat
{
	return NULL;
}

/*
** This is a replacement for the amiga.lib kprintf for PowerPC.
** It uses the WarpUp SPrintF function to do debug output
** to the serial connector (or Sushi, if it's installed).
*/

#if 0
int kprintf(char *format, ...)
{
	int n;

	char msg[1024];
	va_list marker;

	va_start(marker, format);
	n = vsnprintf(msg, 1000, format, marker);
	va_end(marker);

	//IExec->DebugPrintF("%s", msg);

	return n;
}
#else
int kprintf(char *format, ...)
{
	return 0;
}
#endif
