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
#include "amiga_local.h"

#if 0

#include <sys/stat.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <pwd.h>

#include <proto/exec.h>
#include <proto/timer.h>

#else

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <math.h>
//#include <sys/time.h> // Cowcat

#ifdef __VBCC__
#pragma amiga-align
#elif defined(WARPUP)
#pragma pack(2)
#endif

#include <utility/tagitem.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <proto/exec.h>
#include <proto/dos.h>

#ifdef __PPC__
#if defined(__GNUC__)
#include <powerpc/powerpc.h>
#include <powerpc/powerpc_protos.h>
#else
#include <powerpc/powerpc.h>
#include <proto/powerpc.h>
#endif
#endif

#ifndef __PPC__
#include <inline/timer_protos.h>
#endif

#ifdef __VBCC__
#pragma default-align
#elif defined (WARPUP)
#pragma pack()
#endif

#endif

#if defined(__GNUC__)
//typedef void *DIR;
#endif

DIR *findhandle = NULL;


// Used to determine CD Path
static char cdPath[MAX_OSPATH];

// Used to determine local installation path
static char installPath[MAX_OSPATH];

// Used to determine where to store user-specific files
static char homePath[MAX_OSPATH] = { 0 }; // cowcat

qboolean Sys_RandomBytes( byte *string, int len )
{
	return qfalse;
}

#if 0

static struct Device *TimerBase = NULL;
static struct timeval sys_timeBase;

int Sys_Milliseconds(void)
{
	struct timeval curtime;
	static qboolean initialized = qfalse;
	
	if (!initialized)
	{
		#ifdef __PPC__

		GetSysTimePPC(&sys_timeBase);

		#else

		if (!TimerBase)
			TimerBase=(struct Device *)FindName(&SysBase->DeviceList,"timer.device");

		GetSysTime(&sys_timeBase);

		#endif

		initialized = qtrue;
	}
	
	#ifdef __PPC__

	GetSysTimePPC(&curtime);
	SubTimePPC(&curtime, &sys_timeBase);

	#else
	
	GetSysTime(&curtime);
	SubTime(&curtime, &sys_timeBase);

	#endif
	
	return curtime.tv_secs * 1000 + curtime.tv_micro/1000;
}

#else // Cowcat

#if defined(__amiga__) && !defined(__PPC__)
extern struct Library *TimerBase;
#endif

static unsigned int inittime = 0L;

int Sys_Milliseconds(void)
{
	struct timeval 	tv;
  	unsigned int 	currenttime;
	
  	#ifdef __PPC__

  	GetSysTimePPC(&tv);

	#else
	
	static struct Device *TimerBase;

	if (!TimerBase)
		TimerBase = (struct Device *)FindName(&SysBase->DeviceList,"timer.device");

 	GetSysTime(&tv);

	#endif

  	currenttime = tv.tv_secs;

	if (!inittime)
		inittime = currenttime;

  	currenttime = currenttime-inittime;

  	return currenttime * 1000 + tv.tv_micro / 1000;
}

#endif

//#if defined(AMIGA) && 
#if defined(__VBCC__) && defined(__PPC__)
extern float rint(float x);
#endif

#if 0
float rint-test(float x)
{
	static const float TWO23 = 8388608.0;

	if(fabs(x) < TWO23)
	{
		if( x > 0.0 )
		{
			x += TWO23;
			x -= TWO23;

			if(x == 0.0)
				x = 0.0;
		}

		else if (x < 0.0)
		{
			x = TWO23 - x;
			x = -(x - TWO23);

			if(x == 0.0)
				x = -0.0;
		}
		
	}

	return x;
}
#endif

//#include "../renderer/tr_local.h"

void Sys_SnapVector(float *v)
{
	v[0] = rint(v[0]);
	v[1] = rint(v[1]);
	v[2] = rint(v[2]);

	//ri.Printf(PRINT_ALL, "rint 0 %f, 1 %f, 2 %f\n", v[0],v[1],v[2]);
}

void Sys_Mkdir(const char *path)
{
	mkdir (path, 0777);
}


#define	MAX_FOUND_FILES	0x1000

// bk001129 - new in 1.26
void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **list, int *numfiles ) 
{
	char		search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char		filename[MAX_OSPATH];
	DIR		*fdir;
	struct dirent 	*d;
	struct stat 	st;

	if ( *numfiles >= MAX_FOUND_FILES - 1 )
	{
		return;
	}

	if (strlen(subdirs))
	{
		Com_sprintf( search, sizeof(search), "%s/%s", basedir, subdirs );
	}

	else
	{
		Com_sprintf( search, sizeof(search), "%s", basedir );
	}

	if ((fdir = opendir(search)) == NULL) {
		return;
	}

	while ((d = readdir(fdir)) != NULL)
	{
		if (search[strlen(search)-1] == '/')
			Com_sprintf(filename, sizeof(filename), "%s%s", search, d->d_name);

		else
			Com_sprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
			
		if (stat(filename, &st) == -1)
			continue;

		if (st.st_mode & S_IFDIR)
		{
			if (Q_stricmp(d->d_name, ".") && Q_stricmp(d->d_name, ".."))
			{
				if (strlen(subdirs))
				{
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
				}

				else
				{
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
				}

				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}

		if ( *numfiles >= MAX_FOUND_FILES - 1 )
		{
			break;
		}

		Com_sprintf( filename, sizeof(filename), "%s/%s", subdirs, d->d_name );

		if (!Com_FilterPath( filter, filename, qfalse ))
			continue;

		list[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	}

	closedir(fdir);
}

char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs )
{
	struct dirent	*d;
	DIR		*fdir;
	qboolean 	dironly = wantsubs;
	char		search[MAX_OSPATH];
	int		nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];
	int		i;
	struct stat 	st;

	int		extLen;

	if (filter)
	{
		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = NULL; // was 0 - Cowcat
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );

		for ( i = 0 ; i < nfiles ; i++ )
		{
			listCopy[i] = list[i];
		}

		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension)
		extension = "";

	if ( extension[0] == '/' && extension[1] == 0 )
	{
		extension = "";
		dironly = qtrue;
	}

	extLen = strlen( extension );
	
	// search
	nfiles = 0;
	
	if ((fdir = opendir(directory)) == NULL)
	{
		*numfiles = 0;
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL)
	{
		if (directory[strlen(directory)-1] == '/')
			Com_sprintf(search, sizeof(search), "%s%s", directory, d->d_name);

		else
			Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);

		if (stat(search, &st) == -1)
			continue;

		if ((dironly && !(st.st_mode & S_IFDIR)) || (!dironly && (st.st_mode & S_IFDIR)))
			continue;

		if (*extension)
		{
			if ( strlen( d->d_name ) < strlen( extension ) ||
				Q_stricmp( d->d_name + strlen( d->d_name ) - strlen( extension ), extension ) )
			{
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 )
			break;

		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = NULL; // was 0 - Cowcat

	closedir(fdir);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles )
	{
		return NULL;
	}

	listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );

	for ( i = 0 ; i < nfiles ; i++ )
	{
		listCopy[i] = list[i];
	}

	listCopy[i] = NULL;

	return listCopy;
}

void Sys_FreeFileList( char **list ) 
{
	int	i;

	if ( !list )
	{
		return;
	}

	for ( i = 0 ; list[i] ; i++ )
	{
		Z_Free( list[i] );
	}

	Z_Free( list );
}

char *Sys_Cwd( void ) 
{
	static char cwd[MAX_OSPATH];

	getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}


void Sys_SetDefaultCDPath(const char *path)
{
	Q_strncpyz(cdPath, path, sizeof(cdPath));
}

char *Sys_DefaultCDPath(void)
{
	return cdPath;
}

void Sys_SetDefaultInstallPath(const char *path)
{
	Q_strncpyz(installPath, path, sizeof(installPath));
}

char *Sys_DefaultInstallPath(void)
{
	if (*installPath)
		return installPath;

	else
		return Sys_Cwd();
}

void Sys_SetDefaultHomePath(const char *path)
{
	Q_strncpyz(homePath, path, sizeof(homePath));
}

char *Sys_DefaultHomePath(void)
{
#if 0
	char *p;

	if (*homePath)
		return homePath;

// don't  use $HOME because cygwin users complain
//#if 0
	if ((p = getenv("HOME")) != NULL)
	{
		Q_strncpyz(homePath, p, sizeof(homePath));

#ifdef OPEN_ARENA
		Q_strcat(homePath, sizeof(homePath), "/openarena");
#else
		Q_strcat(homePath, sizeof(homePath), "/q3a");
#endif

		if (mkdir(homePath, 0777))
		{
			if (errno != EEXIST) 
				Sys_Error("Unable to create directory \"%s\", error is %s(%d)\n", homePath, strerror(errno), errno);
		}

		return homePath;
	}
#endif

	return ""; // assume current dir
}

//============================================

#if 0
int Sys_GetProcessorId( void )
{
	return CPUID_GENERIC;
}
#endif

cpuFeatures_t Sys_GetProcessorFeatures( void )
{
	return 0; //TODO: should return altivec if available
}

/*
char *Sys_GetCurrentUser( void )
{
	struct passwd *p;

	//if ( (p = getpwuid( getuid() )) == NULL )
	{
		return "player";
	}
	return p->pw_name;
	
}
*/