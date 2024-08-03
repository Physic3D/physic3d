/*
crashhandler.c - advanced crashhandler
Copyright (C) 2016 Mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"

/*
================
Sys_Crash

Crash handler, called from system
================
*/

#pragma comment( lib, "dbghelp" )
#pragma comment( lib, "psapi" )
#include <winnt.h>
#include <dbghelp.h>
#include <psapi.h>

#ifndef XASH_SDL
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
#endif

int ModuleName( HANDLE process, char *name, void *address, int len )
{
	DWORD_PTR   baseAddress = 0;
	static HMODULE     *moduleArray;
	static uint32_t moduleCount;
	LPBYTE      moduleArrayBytes;
	DWORD       bytesRequired;
	int i;

	if(len < 3)
		return 0;

	if ( !moduleArray && EnumProcessModules( process, NULL, 0, &bytesRequired ) )
	{
		if ( bytesRequired )
		{
			moduleArrayBytes = (LPBYTE)LocalAlloc( LPTR, bytesRequired );

			if ( moduleArrayBytes )
			{
				if( EnumProcessModules( process, (HMODULE *)moduleArrayBytes, bytesRequired, &bytesRequired ) )
				{
					moduleCount = bytesRequired / sizeof( HMODULE );
					moduleArray = (HMODULE *)moduleArrayBytes;
				}
			}
		}
	}

	for( i = 0; i<moduleCount; i++ )
	{
		MODULEINFO info;
		GetModuleInformation( process, moduleArray[i], &info, sizeof(MODULEINFO) );

		if( ( address > info.lpBaseOfDll ) &&
				( (DWORD64)address < (DWORD64)info.lpBaseOfDll + (DWORD64)info.SizeOfImage ) )
			return GetModuleBaseName( process, moduleArray[i], name, len );
	}
	return Q_snprintf(name, len, "???");
}
static void stack_trace( PEXCEPTION_POINTERS pInfo )
{
	char message[1024];
	int len = 0;
	size_t i;
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();
	IMAGEHLP_LINE64 line;
	DWORD dline = 0;
	DWORD options;
	CONTEXT context;
	STACKFRAME64 stackframe;
	DWORD image;

	memcpy( &context, pInfo->ContextRecord, sizeof(CONTEXT) );
	options = SymGetOptions();
	options |= SYMOPT_DEBUG;
	options |= SYMOPT_LOAD_LINES;
	SymSetOptions( options );

	SymInitialize( process, NULL, TRUE );



	ZeroMemory( &stackframe, sizeof(STACKFRAME64) );

#ifdef _M_IX86
	image = IMAGE_FILE_MACHINE_I386;
	stackframe.AddrPC.Offset = context.Eip;
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Ebp;
	stackframe.AddrFrame.Mode = AddrModeFlat;
	stackframe.AddrStack.Offset = context.Esp;
	stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
	image = IMAGE_FILE_MACHINE_AMD64;
	stackframe.AddrPC.Offset = context.Rip;
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Rsp;
	stackframe.AddrFrame.Mode = AddrModeFlat;
	stackframe.AddrStack.Offset = context.Rsp;
	stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
	image = IMAGE_FILE_MACHINE_IA64;
	stackframe.AddrPC.Offset = context.StIIP;
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.IntSp;
	stackframe.AddrFrame.Mode = AddrModeFlat;
	stackframe.AddrBStore.Offset = context.RsBSP;
	stackframe.AddrBStore.Mode = AddrModeFlat;
	stackframe.AddrStack.Offset = context.IntSp;
	stackframe.AddrStack.Mode = AddrModeFlat;
#endif
	len += Q_snprintf( message + len, 1024 - len, "Sys_Crash: address %p, code %p\n", pInfo->ExceptionRecord->ExceptionAddress, (void*)pInfo->ExceptionRecord->ExceptionCode );
	if( SymGetLineFromAddr64( process, (DWORD64)pInfo->ExceptionRecord->ExceptionAddress, &dline, &line ) )
	{
		len += Q_snprintf(message + len, 1024 - len,"Exception: %s:%d:%d\n", (char*)line.FileName, (int)line.LineNumber, (int)dline);
	}
	if( SymGetLineFromAddr64( process, stackframe.AddrPC.Offset, &dline, &line ) )
	{
		len += Q_snprintf(message + len, 1024 - len,"PC: %s:%d:%d\n", (char*)line.FileName, (int)line.LineNumber, (int)dline);
	}
	if( SymGetLineFromAddr64( process, stackframe.AddrFrame.Offset, &dline, &line ) )
	{
		len += Q_snprintf(message + len, 1024 - len,"Frame: %s:%d:%d\n", (char*)line.FileName, (int)line.LineNumber, (int)dline);
	}
	for( i = 0; i < 25; i++ )
	{
		char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
		BOOL result = StackWalk64(
			image, process, thread,
			&stackframe, &context, NULL,
			SymFunctionTableAccess64, SymGetModuleBase64, NULL);

		DWORD64 displacement = 0;
		if( !result )
			break;


		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		len += Q_snprintf( message + len, 1024 - len, "% 2d %p", i, (void*)stackframe.AddrPC.Offset );
		if( SymFromAddr( process, stackframe.AddrPC.Offset, &displacement, symbol ) )
		{
			len += Q_snprintf( message + len, 1024 - len, " %s ", symbol->Name );
		}
		if( SymGetLineFromAddr64( process, stackframe.AddrPC.Offset, &dline, &line ) )
		{
			len += Q_snprintf(message + len, 1024 - len,"(%s:%d:%d) ", (char*)line.FileName, (int)line.LineNumber, (int)dline);
		}
		len += Q_snprintf( message + len, 1024 - len, "(");
		len += ModuleName( process, message + len, (void*)stackframe.AddrPC.Offset, 1024 - len );
		len += Q_snprintf( message + len, 1024 - len, ")\n");
	}
#ifdef XASH_SDL
	if( host.type != HOST_DEDICATED ) // let system to restart server automaticly
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,"Sys_Crash", message, host.hWnd );
#endif
	Sys_PrintLog(message);

	SymCleanup(process);
}

LPTOP_LEVEL_EXCEPTION_FILTER       oldFilter;
long _stdcall Sys_Crash( PEXCEPTION_POINTERS pInfo )
{
	// save config
	if( host.state != HOST_CRASHED )
	{
		// check to avoid recursive call
		host.crashed = true;

		Sys_Warn( "Sys_Crash: call %p at address %p", pInfo->ExceptionRecord->ExceptionAddress, pInfo->ExceptionRecord->ExceptionCode );
		stack_trace( pInfo );

		if( host.type == HOST_NORMAL )
			CL_Crashed(); // tell client about crash
		else host.state = HOST_CRASHED;

		if( host.developer <= 0 )
		{
			// no reason to call debugger in release build - just exit
			Sys_Quit();
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		// all other states keep unchanged to let debugger find bug
		Sys_DestroyConsole();
	}

	if( oldFilter )
		return oldFilter( pInfo );
	return EXCEPTION_CONTINUE_EXECUTION;
}

void Sys_SetupCrashHandler( void )
{
	SetErrorMode( SEM_FAILCRITICALERRORS );	// no abort/retry/fail errors
	oldFilter = SetUnhandledExceptionFilter( Sys_Crash );
	host.hInst = GetModuleHandle( NULL );
}

void Sys_RestoreCrashHandler( void )
{
	// restore filter
	if( oldFilter ) SetUnhandledExceptionFilter( oldFilter );
}
