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

#pragma pack(push,2)

#include <exec/exec.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>

#pragma pack(pop)

#ifdef DLL
#include "dll.h"
#endif

struct Library *SocketBase;

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


#ifdef DLL
char *Sys_GetDLLName( const char *name )
{
	return va("%s" ARCH_STRING DLL_EXT, name);
}
#endif

//void * QDECL Sys_LoadDll( const char *name, intptr_t(**entryPoint)(int, ...), intptr_t(*systemcalls)(intptr_t, ...) )
void *Sys_LoadDll( const char *name, char *fqpath, intptr_t(**entryPoint)(int, int, int, int ), intptr_t(*systemcalls)(intptr_t, ...) ) // Cowcat
{
#ifdef DLL

	void	*libHandle;
	char	fname[MAX_OSPATH];
	char	*basepath;
	char	*gamedir;
	char	*fn;
	void	(*dllEntry)( intptr_t (*syscallptr)(intptr_t, ...) );

	Q_strncpyz(fname, Sys_GetDLLName(name), sizeof(fname));

	basepath = Cvar_VariableString( "fs_basepath" );
	gamedir = Cvar_VariableString( "fs_game" );

	//fn = FS_BuildOSPath( basepath, "", fname ); // fuck it - Cowcat
	fn = (char*)name;

	//Com_Printf( "name(%s)... \n", name );
	//Com_Printf( "fname(%s)... \n", fname );

	Com_Printf( "Sys_LoadDll(%s)... \n", fn );
	libHandle = dllLoadLibrary(fn, (char*)name );

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

	return libHandle;

#else

	return NULL;

#endif
}


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

void LeaveAmigaLibs(void)
{
	if(SocketBase)
	{
		CloseLibrary(SocketBase);
		SocketBase = NULL;
	}
}
	
void Sys_Exit(int ex)
{
	//Sys_DestroyConsole();

	NET_Shutdown();

	LeaveAmigaLibs();

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
	char		string[1024];

	va_start (argptr, error);
	Q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	//CL_Shutdown(string, qtrue); // new Cowcat

	//Conbuf_AppendText( text ); // Cowcat
	//Sys_ShowConsole( 1, qtrue ); //

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
	Sys_Exit(0);
}


/*
==============
Sys_Print
==============
*/
void Sys_Print( const char *msg ) 
{
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
	IN_Restart();
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
	//char *cpuidstr;
	
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

	//IN_Init(); // now in amiga_glimp - Cowcat
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


//static char __attribute__((used)) stackcookie[] = "$STACK:2000000";

#ifdef __VBCC__
unsigned long __stack = 0x200000; // auto stack Cowcat
#endif

int main(int argc, char **argv)
{
	char 	*cmdline;
	int 	i, len;
	
	if(SocketBase == NULL)
		SocketBase = OpenLibrary("bsdsocket.library",0L);

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

	Com_Init(cmdline);

	free(cmdline);
	cmdline = NULL;

	NET_Init();

	while( 1 ) 
	{
		// make sure mouse and joystick are only called once a frame
		//IN_Frame(); // now called in common.c - Com_Frame

		// run the game
		Com_Frame();
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

