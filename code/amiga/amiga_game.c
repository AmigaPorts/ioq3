#include <stdlib.h>
#include "dll.h"

#include "../game/g_local.h"

#if 0
void* dllFindResource(int id, char *pType)
{
	return NULL;
}

void* dllLoadResource(void *pHandle)
{
	return NULL;
}

void dllFreeResource(void *pHandle)
{
	return;
}
#endif
	
//intptr_t vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 );
__saveds intptr_t vmMain( int command, int arg0, int arg1, int arg2 );
__saveds void dllEntry( intptr_t (QDECL *syscallptr)( intptr_t arg,... ) );

dll_tExportSymbol DLL_ExportSymbols[]=
{
	//{dllFindResource,"dllFindResource"},
	//{dllLoadResource,"dllLoadResource"},
	//{dllFreeResource,"dllFreeResource"},
	{(intptr_t *)vmMain, "vmMain"},
	{(void *)dllEntry, "dllEntry"},
	{0, 0}
};

dll_tImportSymbol DLL_ImportSymbols[] =
{
	{0, 0, 0, 0}
};

int DLL_Init()
{
	return 1;
}

void DLL_DeInit()
{
}

extern int main(int, char **);

