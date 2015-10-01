// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"

/// header file
#ifndef WD_HOOK_H
#define WD_HOOK_H

extern "C" {

#ifdef WD_HOOK_EXPORTS
#define WD_HOOK_API __declspec(dllexport)
#else
#define WD_HOOK_API __declspec(dllimport)
#endif
	WD_HOOK_API LRESULT hookProc(int code, WPARAM wParam, LPARAM lParam);
}

#endif   // WD_HOOK_H

// Shared data
// Seen by both the instance of this DLL mapped into the remote
// process as well as the instance mapped into our exe
#pragma data_seg (".shared")
#pragma comment(linker,"/section:.shared,rws")
HHOOK hook = NULL;
#pragma data_seg ()

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MessageBox(NULL, L"DLL attached", L"DLL attached", MB_OK);
		break;
	case DLL_PROCESS_DETACH:
		MessageBox(NULL, L"DLL detached", L"DLL detached", MB_OK);
		break;
	}
	return TRUE;
}

#ifndef WD_HOOK_EXPORTS
// TODO: Catch messages here and do stuff with them
LRESULT hookProc(int code, WPARAM wParam, LPARAM lParam) {
	return CallNextHookEx(hook, code, wParam, lParam);
}
#endif