// dll-injector-sample.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//
#include "stdafx.h"

/// header file
#ifndef WD_HOOK_EXPORTS
#define WD_HOOK_EXPORTS

extern "C"
{
#ifdef WD_HOOK_EXPORTS
#define WD_HOOK_API __declspec(dllexport)
#else
#define WD_HOOK_API __declspec(dllimport)
#endif
	WD_HOOK_API LRESULT hookProc(int code, WPARAM wParam, LPARAM lParam);
}

#endif // WD_HOOK_H
bool attached = false;
void createThreadInjection();
void removeThreadInjection();

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		createThreadInjection();
		break;
	case DLL_PROCESS_DETACH:
		removeThreadInjection();
		break;
	}
	return TRUE;
}

LRESULT hookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (!attached)
	{
		createThreadInjection();
	}
	attached = true;

	return CallNextHookEx(nullptr, code, wParam, lParam);
}

void createThreadInjection()
{
}

void removeThreadInjection()
{
}
